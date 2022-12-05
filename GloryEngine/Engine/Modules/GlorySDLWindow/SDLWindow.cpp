#include "SDLWindow.h"
#include "SDLWindowExceptions.h"
#include "Game.h"
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>

namespace Glory
{
	GLORY_API void SDLWindow::GetVulkanSurface(void* instance, void* surface)
	{
		if (!SDL_Vulkan_CreateSurface(m_pWindow, static_cast<VkInstance>(instance), (VkSurfaceKHR*)surface))
		{
			const char* error = SDL_GetError();
			std::cout << "Could not create a Vulkan surface. ERROR: " << error << std::endl;
			throw new SDLErrorException(error);
		}
	}

	GLORY_API void SDLWindow::GetDrawableSize(int* width, int* height)
	{
		SDL_GL_GetDrawableSize(m_pWindow, width, height);
	}

	GLORY_API void SDLWindow::GetWindowSize(int* width, int* height)
	{
		SDL_GetWindowSize(m_pWindow, width, height);
	}

	GLORY_API void SDLWindow::GetWindowPosition(int* x, int* y)
	{
		SDL_GetWindowPosition(m_pWindow, x, y);
	}

	GLORY_API void SDLWindow::SetupForOpenGL()
	{
		// Create OpenGL context
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
		m_GLSDLContext = SDL_GL_CreateContext(m_pWindow);
		if (m_GLSDLContext == nullptr)
		{
			std::cerr << "Could not ceate SDL GL Context: " << SDL_GetError() << std::endl;
			return;
		}

		if (SDL_GL_SetSwapInterval(0) < 0)
		{
			std::cerr << "Could not set SDL GL Swap interval: " << SDL_GetError() << std::endl;
			return;
		}
	}

	GLORY_API void SDLWindow::CleanupOpenGL()
	{
		SDL_GL_DeleteContext(m_GLSDLContext);
		m_GLSDLContext = nullptr;
	}

	GLORY_API void SDLWindow::GLSwapWindow()
	{
		SDL_GL_SwapWindow(m_pWindow);
	}

	GLORY_API void SDLWindow::MakeGLContextCurrent()
	{
		SDL_GL_MakeCurrent(m_pWindow, m_GLSDLContext);
	}

	SDL_Window* SDLWindow::GetSDLWindow()
	{
		return m_pWindow;
	}

	SDL_GLContext SDLWindow::GetSDLGLConext() const
	{
		return m_GLSDLContext;
	}

	GLORY_API void SDLWindow::Resize(int width, int height)
	{
		m_Width = width;
		m_Height = height;
		SDL_SetWindowSize(m_pWindow, width, height);
	}
	
	GLORY_API void SDLWindow::GetPosition(int* width, int* height)
	{
		SDL_GetWindowPosition(m_pWindow, width, height);
	}
	
	GLORY_API void SDLWindow::SetPosition(int width, int height)
	{
		SDL_SetWindowPosition(m_pWindow, width, height);
	}

	SDLWindow::SDLWindow(const WindowCreateInfo& createInfo) : Window(createInfo), m_pWindow(nullptr), m_GLSDLContext(NULL) {}

	SDLWindow::~SDLWindow()
	{
		m_pWindow = NULL;
	}

	GLORY_API void SDLWindow::Open()
	{
		// Create an SDL window that supports Vulkan rendering.
		m_pWindow = SDL_CreateWindow(m_WindowName.c_str(), SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, m_Width, m_Height, m_WindowFlags);

		if (m_pWindow == NULL) throw new SDLErrorException(SDL_GetError());
	}

	GLORY_API void SDLWindow::Close()
	{
		SDL_DestroyWindow(m_pWindow);
	}

	GLORY_API void SDLWindow::PollEvents()
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

	GLORY_API void SDLWindow::GetVulkanRequiredExtensions(std::vector<const char*>& extensions)
	{
		uint32_t extensionCount;
		if (!SDL_Vulkan_GetInstanceExtensions(m_pWindow, &extensionCount, NULL))
			throw new GetVulkanExtensionsException();

		size_t currentSize = extensions.size();
		extensions.resize(currentSize + extensionCount);
		if (!SDL_Vulkan_GetInstanceExtensions(m_pWindow, &extensionCount, &extensions[currentSize]))
			throw new GetVulkanExtensionsException();
	}
}
