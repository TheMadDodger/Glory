#include "EditorOpenGLRenderImpl.h"

#include <Debug.h>
#include <EditorApplication.h>
#include <GL/glew.h>

#include <OpenGLDevice.h>
#include <EditorShaderData.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Glory::Editor
{
	constexpr std::string_view Int64ExtensionLine = "\n#extension GL_ARB_gpu_shader_int64 : require\n";
	constexpr std::string_view BindlessExtensionLine = "\n#extension GL_ARB_bindless_texture : require\n";
	constexpr std::string_view TextureArraySrc = "Textures2D[1024];";
	constexpr std::string_view TextureArrayDst =
		"layout(binding = 9, std430) readonly buffer Textures2DSSBO\n"
		"{\n"
		"    uvec2 Textures2D[];\n"
		"};\n";
	constexpr std::string_view BindlessTextureAccessLine = "texture(Textures2D[";
	constexpr std::string_view AccessInsertSuffix = " : vec4(1.0)";
	constexpr std::string_view MissingTextureColor = "vec4(1.0, 0.753, 0.796, 1.0)";

	EditorOpenGLRenderImpl::EditorOpenGLRenderImpl() {}

	EditorOpenGLRenderImpl::~EditorOpenGLRenderImpl() {}

	std::string EditorOpenGLRenderImpl::ShadingLanguage()
	{
		return "glsl";
	}

	void EditorOpenGLRenderImpl::CompileShaderForEditor(const EditorShaderData& editorShader, std::vector<char>& out)
	{
		spirv_cross::CompilerGLSL compiler(editorShader.Data(), editorShader.Size());
		std::string compiledShader = compiler.compile();

		const bool isBindless = editorShader.HasDefine("ENABLE_BINDLESS");

		if (isBindless)
		{
			const size_t textureArrayIndex = compiledShader.find(TextureArraySrc);
			if (textureArrayIndex != std::string::npos)
			{
				const size_t textureArrayLineStart = compiledShader.rfind('\n', textureArrayIndex) + 1;
				const size_t textureArrayLineEnd = compiledShader.find('\n', textureArrayLineStart);
				const size_t textureArrayLineLength = textureArrayLineEnd - textureArrayLineStart;
				compiledShader.replace(textureArrayLineStart, textureArrayLineLength, TextureArrayDst, 0, TextureArrayDst.size());

				const size_t versionIndex = compiledShader.find("#version");
				const size_t nextLineIndex = compiledShader.find('\n', versionIndex) + 1;
				compiledShader.insert(nextLineIndex, BindlessExtensionLine);
				compiledShader.insert(nextLineIndex, Int64ExtensionLine);
			}

			size_t accessLine = compiledShader.find(BindlessTextureAccessLine);
			while (accessLine != std::string::npos)
			{
				const size_t startLineIndex = compiledShader.rfind('\n', accessLine) + 1;
				const size_t endLineIndex = compiledShader.find('\n', startLineIndex);
				const size_t startId = compiledShader.find('[', accessLine) + 1;
				const size_t endId = compiledShader.rfind(']', endLineIndex);
				const size_t assignmentIndex = compiledShader.rfind('=', accessLine) - 1;
				const size_t coordIdNameIndex = compiledShader.find(',', accessLine) + 1;
				const size_t coordIdNameEndIndex = compiledShader.find(')', coordIdNameIndex);
				const std::string_view prefix{ &compiledShader[startId], endId - startId };
				const std::string_view assignment{ &compiledShader[startLineIndex], assignmentIndex - startLineIndex };
				const std::string_view coordIdName{ &compiledShader[coordIdNameIndex], coordIdNameEndIndex - coordIdNameIndex };
				
				std::stringstream blockBuilder;
				blockBuilder << "uvec2 handle = Textures2D[" << prefix << "];" << std::endl;
				blockBuilder << "sampler2D tex = sampler2D(handle);" << std::endl;
				blockBuilder << assignment << " = handle.x != 0u || handle.y != 0u ? " <<
					" texture(tex, " << coordIdName << ") : " << MissingTextureColor << "; " << std::endl;
				const std::string block = blockBuilder.str();
				compiledShader.replace(startLineIndex, endLineIndex - startLineIndex, block, 0, block.size());
				accessLine = compiledShader.find(BindlessTextureAccessLine, startLineIndex + block.size());
			}
		}

		out.resize(compiledShader.length() + 1);
		std::memcpy(out.data(), compiledShader.data(), compiledShader.length());
		out.back() = '\0';
	}

	const std::vector<std::string>& EditorOpenGLRenderImpl::GetDeviceUniqueDefines() const
	{
		static std::vector<std::string> defines = {
			"DEVICE_OPENGL",
			"NO_PUSH_CONSTANTS"
		};
		return defines;
	}

	void EditorOpenGLRenderImpl::SetContext(ImGuiContext* pImguiConext)
	{
		ImGui::SetCurrentContext(pImguiConext);
	}

	void EditorOpenGLRenderImpl::Setup()
	{
		m_pEditorPlatform->GetWindowImpl()->SetupForOpenGL();
	}

	void EditorOpenGLRenderImpl::SetupBackend()
	{
		const char* glsl_version = "#version 130";
		ImGui_ImplOpenGL3_Init(glsl_version);
	}

	void EditorOpenGLRenderImpl::UploadImGUIFonts()
	{
		
	}

	void EditorOpenGLRenderImpl::Shutdown()
	{
		
	}

	void EditorOpenGLRenderImpl::Cleanup()
	{
		ImGui_ImplOpenGL3_Shutdown();
	}

	void EditorOpenGLRenderImpl::BeforeRender()
	{
		
	}

	void EditorOpenGLRenderImpl::NewFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
	}

	void EditorOpenGLRenderImpl::Clear(const ImVec4& clearColor)
	{
		int display_w, display_h;
		m_pEditorPlatform->GetWindowImpl()->GetMainWindow()->GetDrawableSize(&display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		//glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		//glClear(GL_COLOR_BUFFER_BIT);
	}

	void EditorOpenGLRenderImpl::FrameRender(ImDrawData* pDrawData)
	{
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void EditorOpenGLRenderImpl::FramePresent()
	{
		m_pEditorPlatform->GetWindowImpl()->GetMainWindow()->GLSwapWindow();
	}

	void* EditorOpenGLRenderImpl::GetTextureID(TextureHandle texture)
	{
		if (!texture) return 0;
		GraphicsDevice* pDevice = EditorApplication::GetInstance()->GetEngine()->ActiveGraphicsDevice();
		if (!pDevice) return 0;
		OpenGLDevice* pGLDevice = static_cast<OpenGLDevice*>(pDevice);
		return (void*)pGLDevice->GetGLTextureID(texture);
	}

	void EditorOpenGLRenderImpl::LogGLError(const unsigned int& err, bool bIncludeTimeStamp)
	{
		if (err != GL_NO_ERROR)
		{
			const char* error = (const char*)glewGetErrorString(err);
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogWarning(error, bIncludeTimeStamp);
		}
	}

	void LoadBackend(EditorCreateInfo& editorCreateInfo)
	{
		editorCreateInfo.pRenderImpl = new EditorOpenGLRenderImpl();
	}
}
