#include "stdafx.h"
#include "Window.h"
#include "CoreExceptions.h"

namespace Glory
{
	Window::Window(const std::string& windowName) : m_pWindow(nullptr), m_WindowName(windowName)
	{
	}

	Window::~Window()
	{
	}

	void Window::OpenWindow()
	{
		// Create an SDL window that supports Vulkan rendering.
		m_pWindow = SDL_CreateWindow(m_WindowName.c_str(), SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);

		if (m_pWindow == NULL) throw new SDLErrorException(SDL_GetError());

        if (!SDL_Vulkan_GetInstanceExtensions(m_pWindow, &m_ExtensionCount, NULL)) {
            std::cout << "Could not get the number of required instance extensions from SDL." << std::endl;
        }
        m_Extensions = std::vector<const char*>(m_ExtensionCount);
        if (!SDL_Vulkan_GetInstanceExtensions(m_pWindow, &m_ExtensionCount, m_Extensions.data())) {
            std::cout << "Could not get the names of required instance extensions from SDL." << std::endl;
        }

        // Use validation layers if this is a debug build
#if defined(_DEBUG)
        m_Layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

        // vk::ApplicationInfo allows the programmer to specifiy some basic information about the
        // program, which can be useful for layers and tools to provide more debug information.
        vk::ApplicationInfo appInfo = vk::ApplicationInfo()
            .setPApplicationName("Vulkan C++ Windowed Program Template")
            .setApplicationVersion(1)
            .setPEngineName("LunarG SDK")
            .setEngineVersion(1)
            .setApiVersion(VK_API_VERSION_1_0);

        // vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
        // are needed.
        vk::InstanceCreateInfo instInfo = vk::InstanceCreateInfo()
            .setFlags(vk::InstanceCreateFlags())
            .setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount(static_cast<uint32_t>(m_Extensions.size()))
            .setPpEnabledExtensionNames(m_Extensions.data())
            .setEnabledLayerCount(static_cast<uint32_t>(m_Layers.size()))
            .setPpEnabledLayerNames(m_Layers.data());

        // Create the Vulkan instance.
        try {
            m_Instance = vk::createInstance(instInfo);
        }
        catch (const std::exception & e) {
            std::cout << "Could not create a Vulkan instance: " << e.what() << std::endl;
        }

        // Create a Vulkan surface for rendering
        if (!SDL_Vulkan_CreateSurface(m_pWindow, static_cast<VkInstance>(m_Instance), &m_cSurface)) {
            std::cout << "Could not create a Vulkan surface." << std::endl;
        }
        m_Surface = vk::SurfaceKHR(m_cSurface);
	}

    void Window::CloseWindow()
    {
        m_Instance.destroySurfaceKHR(m_Surface);
        SDL_DestroyWindow(m_pWindow);
        m_Instance.destroy();
    }
}