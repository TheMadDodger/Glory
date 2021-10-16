#include "imgui_impl_sdl.h"
#include "EditorSDLWindowImpl.h"
#include <SDLWindow.h>

namespace Glory::Editor
{
	EditorSDLWindowImpl::EditorSDLWindowImpl() {}

	EditorSDLWindowImpl::~EditorSDLWindowImpl() {}

	void EditorSDLWindowImpl::Shutdown()
	{
		ImGui_ImplSDL2_Shutdown();
	}

	void EditorSDLWindowImpl::SetupForOpenGL()
	{
		SDLWindow* pSDLWindow = (SDLWindow*)m_pMainWindow;
		ImGui_ImplSDL2_InitForOpenGL(pSDLWindow->GetSDLWindow(), pSDLWindow->GetSDLGLConext());
	}

	void EditorSDLWindowImpl::SetupForVulkan()
	{
		SDLWindow* pSDLWindow = (SDLWindow*)m_pMainWindow;
		ImGui_ImplSDL2_InitForVulkan(pSDLWindow->GetSDLWindow());
	}

	void EditorSDLWindowImpl::SetupForD3D()
	{
		SDLWindow* pSDLWindow = (SDLWindow*)m_pMainWindow;
		ImGui_ImplSDL2_InitForD3D(pSDLWindow->GetSDLWindow());
	}

	void EditorSDLWindowImpl::SetupForMetal()
	{
		SDLWindow* pSDLWindow = (SDLWindow*)m_pMainWindow;
		ImGui_ImplSDL2_InitForMetal(pSDLWindow->GetSDLWindow());
	}

	bool EditorSDLWindowImpl::PollEvents()
	{
		SDLWindow* pSDLWindow = (SDLWindow*)m_pMainWindow;

		// Poll and handle events (inputs, window resize, etc.)
			// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
			// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
			// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
			// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				return true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(pSDLWindow->GetSDLWindow()))
				return true;
		}

		return false;
	}

	void EditorSDLWindowImpl::NewFrame()
	{
		SDLWindow* pSDLWindow = (SDLWindow*)m_pMainWindow;
		ImGui_ImplSDL2_NewFrame(pSDLWindow->GetSDLWindow());
	}
}
