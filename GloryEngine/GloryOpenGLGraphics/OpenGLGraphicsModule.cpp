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
		
		FileImportSettings importSettings;
		importSettings.Flags = (int)std::ios::ate;
		importSettings.AddNullTerminateAtEnd = true;
		FileData* pVert = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load("./Shaders/vertexbuffertest.vert", importSettings);
		FileData* pFrag = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load("./Shaders/triangle.frag", importSettings);
		
		GLShader* pVertShader = new GLShader(pVert, ShaderType::ST_Vertex, "");
		pVertShader->Initialize();
		GLShader* pFragShader = new GLShader(pFrag, ShaderType::ST_Fragment, "");
		pFragShader->Initialize();
		
		int success;
		char infoLog[512];
		
		m_ShaderProgram = glCreateProgram();
		LogGLError(glGetError());
		glAttachShader(m_ShaderProgram, pVertShader->m_ShaderID);
		LogGLError(glGetError());
		glAttachShader(m_ShaderProgram, pFragShader->m_ShaderID);
		LogGLError(glGetError());
		glLinkProgram(m_ShaderProgram);
		LogGLError(glGetError());
		
		glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
		LogGLError(glGetError());
		if (!success)
		{
			glGetProgramInfoLog(m_ShaderProgram, 512, NULL, infoLog);
			LogGLError(glGetError());
			Debug::LogError(infoLog);
		}
		
		glUseProgram(m_ShaderProgram);
		LogGLError(glGetError());
		
		delete pVertShader;
		delete pFragShader;
		
		delete pVert;
		delete pFrag;
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
		GetEngine()->GetWindowModule()->GetMainWindow()->MakeGLContextCurrent();

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

	void OpenGLGraphicsModule::DrawMesh(MeshData* pMeshData)
	{
		Mesh* pMesh = GetResourceManager()->CreateMesh(pMeshData);
		glUseProgram(m_ShaderProgram);
		LogGLError(glGetError());
		pMesh->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		LogGLError(glGetError());
	}
}
