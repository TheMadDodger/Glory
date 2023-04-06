#include "OpenGLGraphicsModule.h"
#include "VertexHelpers.h"
#include "FileLoaderModule.h"
#include "GLShader.h"
#include "OGLResourceManager.h"
#include "GloryOGL.h"
#include "GLConverter.h"

#include <Engine.h>
#include <Debug.h>
#include <ios>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(OpenGLGraphicsModule, 0, 1);

	OpenGLGraphicsModule::OpenGLGraphicsModule()
		: m_ScreenQuadVertexArrayID(0), m_ScreenQuadVertexbufferID(0)
	{
	}

	OpenGLGraphicsModule::~OpenGLGraphicsModule()
	{
	}

	void OpenGLGraphicsModule::OnInitialize()
	{
	}

	void OpenGLGraphicsModule::OnCleanup()
	{
	}

	void OpenGLGraphicsModule::ThreadedCleanup()
	{
		glDeleteVertexArrays(1, &m_ScreenQuadVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glDeleteBuffers(1, &m_ScreenQuadVertexbufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		m_ScreenQuadVertexArrayID = 0;
		m_ScreenQuadVertexbufferID = 0;

		GraphicsModule::ThreadedCleanup();
		GetEngine()->GetWindowModule()->GetMainWindow()->CleanupOpenGL();
		LogGLError(glGetError());
	}

	void OpenGLGraphicsModule::ThreadedInitialize()
	{
		Window* pMainWindow = GetEngine()->GetWindowModule()->GetMainWindow();
		pMainWindow->SetupForOpenGL();
		LogGLError(glGetError());
		GetEngine()->GetWindowModule()->GetMainWindow()->MakeGLContextCurrent();
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

		int width, height;
		pMainWindow->GetWindowSize(&width, &height);
		glViewport(0, 0, width, height);
		LogGLError(glGetError());

		GLint last_texture, last_array_buffer, last_vertex_array;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

		glCreateShader(GL_VERTEX_SHADER);

		static float vertices[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		};

		glGenVertexArrays(1, &m_ScreenQuadVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindVertexArray(m_ScreenQuadVertexArrayID);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glGenBuffers(1, &m_ScreenQuadVertexbufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadVertexbufferID);
		OpenGLGraphicsModule::LogGLError(glGetError());
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glEnableVertexAttribArray(0);
		OpenGLGraphicsModule::LogGLError(glGetError());

		glBindVertexArray(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	GPUResourceManager* OpenGLGraphicsModule::CreateGPUResourceManager()
	{
		return new OGLResourceManager();
	}

	void OpenGLGraphicsModule::LogGLError(const GLenum& err, bool bIncludeTimeStamp)
	{
		if (err != GL_NO_ERROR)
		{
			const char* error = (const char*)glewGetErrorString(err);
			Debug::LogWarning(error, bIncludeTimeStamp);
		}
	}

	void OpenGLGraphicsModule::Clear(glm::vec4 color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		LogGLError(glGetError());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		LogGLError(glGetError());
	}

	void OpenGLGraphicsModule::Swap()
	{
		Window* pMainWindow = GetEngine()->GetWindowModule()->GetMainWindow();
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
		pMesh->Bind();
		const GLuint primitiveType = GLConverter::TO_GLPRIMITIVETYPE.at(pMesh->GetPrimitiveType());
		const uint32_t indexCount = pMesh->GetIndexCount();
		if (indexCount == 0) glDrawArrays(primitiveType, vertexOffset, vertexCount ? vertexCount : pMesh->GetVertexCount());
		else glDrawElements(primitiveType, indexCount, GL_UNSIGNED_INT, NULL);
		LogGLError(glGetError());
		glBindVertexArray(NULL);
		OpenGLGraphicsModule::LogGLError(glGetError());
	}

	void OpenGLGraphicsModule::DrawScreenQuad()
	{
		glBindVertexArray(m_ScreenQuadVertexArrayID);
		LogGLError(glGetError());

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
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

	void OpenGLGraphicsModule::SetViewport(int x, int y, uint32_t width, uint32_t height)
	{
		glViewport(0, 0, width, height);
	}
}
