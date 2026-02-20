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

	OpenGLGraphicsModule::OpenGLGraphicsModule(): m_Device(this)
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

		m_pEngine->AddGraphicsDevice(&m_Device);
	}

	void OpenGLGraphicsModule::Cleanup()
	{
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
