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
	EditorOpenGLRenderImpl::EditorOpenGLRenderImpl() {}

	EditorOpenGLRenderImpl::~EditorOpenGLRenderImpl() {}

	std::string EditorOpenGLRenderImpl::ShadingLanguage()
	{
		return "glsl";
	}

	void EditorOpenGLRenderImpl::CompileShaderForEditor(const EditorShaderData& editorShader, std::vector<char>& out)
	{
		spirv_cross::CompilerGLSL compiler(editorShader.Data(), editorShader.Size());
		const std::string compiledShader = compiler.compile();
		out.resize(compiledShader.length() + 1);
		std::memcpy(out.data(), compiledShader.data(), compiledShader.length());
		out.back() = '\0';
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
