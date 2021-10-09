#include "OpenGLGraphicsModule.h"
#include <Engine.h>
#include <Debug.h>
#include "VertexHelpers.h"
#include "FileLoaderModule.h"
#include <ios>
#include "GLShader.h"
#include "OGLResourceManager.h"

namespace Glory
{
	OpenGLGraphicsModule::OpenGLGraphicsModule()
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
		GetEngine()->GetWindowModule()->GetMainWindow()->CleanupOpenGL();
	}

	void OpenGLGraphicsModule::ThreadedInitialize()
	{
		Window* pMainWindow = GetEngine()->GetWindowModule()->GetMainWindow();
		pMainWindow->SetupForOpenGL();
		GetEngine()->GetWindowModule()->GetMainWindow()->MakeGLContextCurrent();

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

	void OpenGLGraphicsModule::Clear()
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
		Material* pMaterial = GetResourceManager()->CreateMaterial(pMaterialData);
		pMaterial->Use();
		return pMaterial;
	}

	void OpenGLGraphicsModule::DrawMesh(MeshData* pMeshData)
	{
		Mesh* pMesh = GetResourceManager()->CreateMesh(pMeshData);
		pMesh->Bind();
		glDrawElements(GL_TRIANGLES, pMesh->GetIndexCount(), GL_UNSIGNED_INT, NULL);
		LogGLError(glGetError());
	}
}
