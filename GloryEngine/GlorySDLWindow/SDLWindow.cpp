#include "SDLWindow.h"
#include "SDLWindowExceptions.h"
#include "Game.h"
// GlorySDLWindow.cpp : Defines the functions for the static library.
//

namespace Glory
{
	SDLWindow::SDLWindow(const WindowCreateInfo& createInfo) : Window(createInfo), m_pWindow(nullptr) {}

	SDLWindow::~SDLWindow()
	{
	}

	void SDLWindow::Open()
	{
		// Create an SDL window that supports Vulkan rendering.
		m_pWindow = SDL_CreateWindow(m_WindowName.c_str(), SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, m_Width, m_Height, 0/*SDL_WINDOW_VULKAN*/);

		if (m_pWindow == NULL) throw new SDLErrorException(SDL_GetError());
	}

	void SDLWindow::Close()
	{
		SDL_DestroyWindow(m_pWindow);
	}

	void SDLWindow::PollEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type) {
			
			case SDL_QUIT:
				Game::Quit();
				break;
			
			default:
				// Do nothing.
				break;
			}
		}
	}
}