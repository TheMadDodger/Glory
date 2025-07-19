#include "OpenGLGraphicsModule.h"
#include "VertexHelpers.h"
#include "FileLoaderModule.h"
#include "GLShader.h"
#include "OGLResourceManager.h"
#include "GloryOGL.h"
#include "GLConverter.h"
#include "OGLRenderTexture.h"

#include <Engine.h>
#include <Debug.h>
#include <WindowModule.h>
#include <Debug.h>
#include <ios>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(OpenGLGraphicsModule);

	Engine* EngineInstance;

	OpenGLGraphicsModule::OpenGLGraphicsModule()
		: m_ScreenQuadVertexArrayID(0), m_ScreenQuadVertexbufferID(0)
	{
	}

	OpenGLGraphicsModule::~OpenGLGraphicsModule()
	{
	}

	void OpenGLGraphicsModule::OnInitialize()
	{
		EngineInstance = m_pEngine;

		Window* pMainWindow = GetEngine()->GetMainModule<WindowModule>()->GetMainWindow();
		pMainWindow->SetupForOpenGL();
		LogGLError(glGetError());
		GetEngine()->GetMainModule<WindowModule>()->GetMainWindow()->MakeGLContextCurrent();
		LogGLError(glGetError());

		// Init GLEW
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		}
		LogGLError(glGetError());

		fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
		LogGLError(glGetError());

		// Should be done in the window itself, probably api independant
		//if (SDL_GL_SetSwapInterval(1) < 0)
		//{
		//	std::cerr << "Could not set SDL GL Swap interval: " << SDL_GetError() << std::endl;
		//	return;
		//}
		//SDL_GL_SetSwapInterval(0);

		LogGLError(glGetError());

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		LogGLError(glGetError());

		//// Enable color blending and use alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		LogGLError(glGetError());

		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		LogGLError(glGetError());
		//// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);
		LogGLError(glGetError());

		glEnable(GL_LINE_SMOOTH);
		LogGLError(glGetError());

		glEnable(GL_MULTISAMPLE);
		LogGLError(glGetError());

		int width, height;
		pMainWindow->GetWindowSize(&width, &height);
		glViewport(0, 0, width, height);
		LogGLError(glGetError());

		GLint last_texture, last_array_buffer, last_vertex_array;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

		glCreateShader(GL_VERTEX_SHADER);

		static const float vertices[] = {
			-1.0f, -1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,
		};

		glGenVertexArrays(1, &m_ScreenQuadVertexArrayID);
		LogGLError(glGetError());
		glBindVertexArray(m_ScreenQuadVertexArrayID);
		LogGLError(glGetError());

		glGenBuffers(1, &m_ScreenQuadVertexbufferID);
		LogGLError(glGetError());
		glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadVertexbufferID);
		LogGLError(glGetError());
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		LogGLError(glGetError());

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		LogGLError(glGetError());

		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		LogGLError(glGetError());

		glEnableVertexAttribArray(0);
		LogGLError(glGetError());

		glBindVertexArray(NULL);
		LogGLError(glGetError());

		static const float cubeVertices[] = {
			-1.0f,-1.0f,-1.0f, // triangle 1 : begin
			-1.0f,-1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f, // triangle 1 : end
			1.0f, 1.0f,-1.0f, // triangle 2 : begin
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f,-1.0f, // triangle 2 : end
			1.0f,-1.0f, 1.0f,
			-1.0f,-1.0f,-1.0f,
			1.0f,-1.0f,-1.0f,
			1.0f, 1.0f,-1.0f,
			1.0f,-1.0f,-1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f,-1.0f,
			1.0f,-1.0f, 1.0f,
			-1.0f,-1.0f, 1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f,-1.0f, 1.0f,
			1.0f,-1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f,-1.0f,-1.0f,
			1.0f, 1.0f,-1.0f,
			1.0f,-1.0f,-1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f,-1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f,-1.0f,
			-1.0f, 1.0f,-1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f,-1.0f, 1.0f
		};

		glGenVertexArrays(1, &m_UnitCubeVertexArrayID);
		LogGLError(glGetError());
		glBindVertexArray(m_UnitCubeVertexArrayID);
		LogGLError(glGetError());

		glGenBuffers(1, &m_UnitCubeVertexbufferID);
		LogGLError(glGetError());
		glBindBuffer(GL_ARRAY_BUFFER, m_UnitCubeVertexbufferID);
		LogGLError(glGetError());
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		LogGLError(glGetError());

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
		LogGLError(glGetError());

		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		LogGLError(glGetError());

		glEnableVertexAttribArray(0);
		LogGLError(glGetError());

		glBindVertexArray(NULL);
		LogGLError(glGetError());

		m_pPassthroughMaterial = new MaterialData();
		m_pPassthroughMaterial->SetPipeline(802);
	}

	void OpenGLGraphicsModule::OnCleanup()
	{
		glDeleteVertexArrays(1, &m_ScreenQuadVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glDeleteBuffers(1, &m_ScreenQuadVertexbufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		m_ScreenQuadVertexArrayID = 0;
		m_ScreenQuadVertexbufferID = 0;
		
		glDeleteVertexArrays(1, &m_UnitCubeVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glDeleteBuffers(1, &m_UnitCubeVertexbufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		m_UnitCubeVertexArrayID = 0;
		m_UnitCubeVertexbufferID = 0;

		GetEngine()->GetMainModule<WindowModule>()->GetMainWindow()->CleanupOpenGL();
		LogGLError(glGetError());
	}

	GPUResourceManager* OpenGLGraphicsModule::CreateGPUResourceManager()
	{
		return new OGLResourceManager(m_pEngine);
	}

	void OpenGLGraphicsModule::LogGLError(const GLenum& err, bool bIncludeTimeStamp)
	{
		if (err != GL_NO_ERROR)
		{
			const char* error = (const char*)glewGetErrorString(err);
			EngineInstance->GetDebug().LogWarning(error, bIncludeTimeStamp);
		}
	}

	void OpenGLGraphicsModule::Clear(glm::vec4 color, double depth)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		LogGLError(glGetError());
		glClearDepth(depth);
		LogGLError(glGetError());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		LogGLError(glGetError());
	}

	void OpenGLGraphicsModule::Swap()
	{
		Window* pMainWindow = GetEngine()->GetMainModule<WindowModule>()->GetMainWindow();
		pMainWindow->GLSwapWindow();
	}

	Material* OpenGLGraphicsModule::UseMaterial(MaterialData* pMaterialData)
	{
		glUseProgram(NULL);
		LogGLError(glGetError());
		if (pMaterialData == nullptr) return nullptr;
		Material* pMaterial = GetResourceManager()->CreateMaterial(pMaterialData);
		if (!pMaterial) return nullptr;
		pMaterial->Use();
		return pMaterial;
	}

	void OpenGLGraphicsModule::OnDrawMesh(Mesh* pMesh, uint32_t vertexOffset, uint32_t vertexCount)
	{
		pMesh->BindForDraw();
		const GLuint primitiveType = GLConverter::TO_GLPRIMITIVETYPE.at(pMesh->GetPrimitiveType());
		const uint32_t indexCount = pMesh->GetIndexCount();
		if (indexCount == 0) glDrawArrays(primitiveType, vertexOffset, vertexCount ? vertexCount : pMesh->GetVertexCount());
		else glDrawElements(primitiveType, indexCount, GL_UNSIGNED_INT, NULL);
		LogGLError(glGetError());
		glBindVertexArray(NULL);
		LogGLError(glGetError());
	}

	void OpenGLGraphicsModule::OnMultiDrawMeshIndirect(Mesh* pMesh, size_t count)
	{
		pMesh->BindForDraw();
		const GLuint primitiveType = GLConverter::TO_GLPRIMITIVETYPE.at(pMesh->GetPrimitiveType());
		glMultiDrawElementsIndirect(primitiveType, GL_UNSIGNED_INT, NULL, count, 0);
		LogGLError(glGetError());
	}

	void OpenGLGraphicsModule::DrawScreenQuad()
	{
		glBindVertexArray(m_ScreenQuadVertexArrayID);
		LogGLError(glGetError());

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
		LogGLError(glGetError());
	}

	void OpenGLGraphicsModule::DrawUnitCube()
	{
		glBindVertexArray(m_UnitCubeVertexArrayID);
		LogGLError(glGetError());

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, 36);
		LogGLError(glGetError());
	}

	void OpenGLGraphicsModule::DispatchCompute(size_t num_groups_x, size_t num_groups_y, size_t num_groups_z)
	{
		glDispatchCompute((GLuint)num_groups_x, (GLuint)num_groups_y, (GLuint)num_groups_z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void OpenGLGraphicsModule::EnableDepthTest(bool enable)
	{
		if (enable)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	void OpenGLGraphicsModule::EnableDepthWrite(bool enable)
	{
		glDepthMask(enable);
	}

	void OpenGLGraphicsModule::EnableStencilTest(bool enable)
	{
		if (enable)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
	}

	void OpenGLGraphicsModule::SetStencilMask(unsigned int mask)
	{
		glStencilMask(mask);
	}

	void OpenGLGraphicsModule::SetStencilFunc(CompareOp func, int ref, unsigned int mask)
	{
		const GLenum glFunc = GLConverter::TO_GLOP.at(func);
		glStencilFunc(glFunc, ref, mask);
	}

	void OpenGLGraphicsModule::SetStencilOP(Func fail, Func dpfail, Func dppass)
	{
		const GLenum glFail = GLConverter::TO_GLFUNC.at(fail);
		const GLenum gldpFail = GLConverter::TO_GLFUNC.at(dpfail);
		const GLenum gldpPass = GLConverter::TO_GLFUNC.at(dppass);
		glStencilOp(glFail, gldpFail, gldpPass);
	}

	void OpenGLGraphicsModule::SetColorMask(bool r, bool g, bool b, bool a)
	{
		glColorMask(r, g, b, a);
	}

	void OpenGLGraphicsModule::ClearStencil(int value)
	{
		glClearStencil(value);
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	void OpenGLGraphicsModule::SetViewport(int x, int y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLGraphicsModule::Scissor(int x, int y, uint32_t width, uint32_t height)
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(x, y, width, height);
	}

	void OpenGLGraphicsModule::EndScissor()
	{
		glDisable(GL_SCISSOR_TEST);
	}

	void OpenGLGraphicsModule::Blit(RenderTexture* pTexture, glm::uvec4 src, glm::uvec4 dst, Filter filter)
	{
		GLenum glFilter = GLConverter::TO_GLFILTER.at(filter);

		pTexture->BindRead();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		OpenGLGraphicsModule::LogGLError(glGetError());
		uint32_t width, height;
		pTexture->GetDimensions(width, height);

		if (src.z == 0) src.z = width;
		if (src.w == 0) src.w = height;
		if (dst.z == 0) dst.z = width;
		if (dst.w == 0) dst.w = height;

		glBlitFramebuffer(src.x, src.y, src.z, src.w, dst.x, dst.y, dst.z, dst.w,
			GL_COLOR_BUFFER_BIT, glFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
		pTexture->UnBindRead();
	}

	void OpenGLGraphicsModule::Blit(RenderTexture* pSource, RenderTexture* pDest, glm::uvec4 src, glm::uvec4 dst, Filter filter)
	{
		GLenum glFilter = GLConverter::TO_GLFILTER.at(filter);

		uint32_t srcWidth, srcHeight, dstWidth, dstHeight;
		pSource->GetDimensions(srcWidth, srcHeight);
		pDest->GetDimensions(dstWidth, dstHeight);

		if (src.z == 0) src.z = srcWidth;
		if (src.w == 0) src.z = srcHeight;
		if (dst.z == 0) dst.z = dstWidth;
		if (dst.w == 0) dst.w = dstHeight;

		const GLuint srcID = static_cast<OGLRenderTexture*>(pSource)->ID();
		const GLuint dstID = static_cast<OGLRenderTexture*>(pDest)->ID();

		glBlitNamedFramebuffer(srcID, dstID, src.x, src.y, src.z, src.w, dst.x, dst.y, dst.z, dst.w,
			GL_COLOR_BUFFER_BIT, glFilter);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLGraphicsModule::SetCullFace(CullFace cullFace)
	{
		switch (cullFace)
		{
		case Glory::CullFace::None:
			glDisable(GL_CULL_FACE);
			return;
		case Glory::CullFace::Front:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			return;
		case Glory::CullFace::Back:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			return;
		case Glory::CullFace::FrontAndBack:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT_AND_BACK);
			return;
		}
	}

	Material* OpenGLGraphicsModule::UsePassthroughMaterial()
	{
		return UseMaterial(m_pPassthroughMaterial);
	}
}
