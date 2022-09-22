#include "pch.h"
#include "HubWindow.h"
#include "imgui_impl_sdl.h"

namespace Glory::EditorLauncher
{
	HubWindow::HubWindow(const std::string& windowName, const std::string& glslVersion) : m_WindowName(windowName), m_GLSLVersion(glslVersion), m_pWindow(nullptr), m_GLContext(NULL)
	{
	}

	HubWindow::~HubWindow()
	{
		SDL_GL_DeleteContext(m_GLContext);
		SDL_DestroyWindow(m_pWindow);
		SDL_Quit();
		m_pWindow = nullptr;
	}

	bool HubWindow::Initialize()
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			printf("Error: %s\n", SDL_GetError());
			return false;
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		m_pWindow = SDL_CreateWindow(m_WindowName.c_str(), SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, 1100, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

		m_GLContext = SDL_GL_CreateContext(m_pWindow);
		SDL_GL_MakeCurrent(m_pWindow, m_GLContext);
		SDL_GL_SetSwapInterval(1); // Enable vsync

		return true;
	}

	SDL_Window* HubWindow::GetSDLWindow()
	{
		return m_pWindow;
	}

	SDL_GLContext HubWindow::GetGLContext()
	{
		return m_GLContext;
	}

	const char* HubWindow::GetGLSLVersion()
	{
		return m_GLSLVersion.c_str();
	}

	bool HubWindow::PollEvents()
	{
		bool done = false;
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_pWindow))
				done = true;
		}
		return done;
	}

	void HubWindow::Swap()
	{
		SDL_GL_SwapWindow(m_pWindow);
	}
}
