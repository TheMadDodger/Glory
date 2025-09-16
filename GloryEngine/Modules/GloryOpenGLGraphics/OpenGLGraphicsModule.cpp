#include "OpenGLGraphicsModule.h"
#include "VertexHelpers.h"
#include "FileLoaderModule.h"
#include "GloryOGL.h"
#include "GLConverter.h"

#include <Engine.h>
#include <Window.h>
#include <Debug.h>
#include <WindowModule.h>
#include <Debug.h>
#include <ios>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(OpenGLGraphicsModule);

	Engine* EngineInstance;

	OpenGLGraphicsModule::OpenGLGraphicsModule()
		: m_ScreenQuadVertexArrayID(0), m_ScreenQuadVertexbufferID(0), m_Device(this)
	{
	}

	OpenGLGraphicsModule::~OpenGLGraphicsModule()
	{
	}

	void OpenGLGraphicsModule::PreInitialize()
	{
		m_pEngine->MainWindowInfo().WindowFlags |= W_OpenGL;
	}

	void OpenGLGraphicsModule::Initialize()
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

		//m_pPassthroughMaterial = new MaterialData();
		//m_pPassthroughMaterial->SetPipeline(802);

		m_pEngine->AddGraphicsDevice(&m_Device);
	}

	void OpenGLGraphicsModule::Cleanup()
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

	void OpenGLGraphicsModule::LogGLError(const GLenum& err, bool bIncludeTimeStamp)
	{
		if (err != GL_NO_ERROR)
		{
			const char* error = (const char*)glewGetErrorString(err);
			EngineInstance->GetDebug().LogWarning(error, bIncludeTimeStamp);
		}
	}

	const std::type_info& OpenGLGraphicsModule::GetModuleType()
	{
		return typeid(OpenGLGraphicsModule);
	}
}
