#include "imgui_impl_sdl.h"
#include "EditorSDLWindowImpl.h"
#include <SDLWindow.h>
#include <EditorWindow.h>
#include <GameWindow.h>

namespace Glory::Editor
{
	GLORY_API void LoadBackend(EditorCreateInfo& editorCreateInfo)
	{
		editorCreateInfo.pWindowImpl = new EditorSDLWindowImpl();
	}

	EditorSDLWindowImpl::EditorSDLWindowImpl() : m_AltIsDown(false) {}

	EditorSDLWindowImpl::~EditorSDLWindowImpl() {}

	void EditorSDLWindowImpl::SetContext(ImGuiContext* pImguiConext)
	{
		ImGui::SetCurrentContext(pImguiConext);
	}

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
		while (pSDLWindow->PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LALT)
				m_AltIsDown = true;
			else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LALT)
				m_AltIsDown = false;

			EditorWindow* gameWindow = EditorWindow::FindEditorWindow(typeid(GameWindow));
			if (gameWindow && gameWindow->IsFocused() && !m_AltIsDown && pSDLWindow->HandleInputEvents(event)) continue;

			if (ImGui_ImplSDL2_ProcessEvent(&event)) continue;
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
