#pragma once
#include "Object.h"

namespace Glory
{
	class Window : public Object
	{
	public:
		Window(const std::string& windowName);
		virtual ~Window();

		void OpenWindow();
		void CloseWindow();

	private:
		SDL_Window* m_pWindow;
		std::string m_WindowName;

		unsigned m_ExtensionCount;
		std::vector<const char*> m_Extensions;
		std::vector<const char*> m_Layers;
		vk::Instance m_Instance;
		VkSurfaceKHR m_cSurface;
		vk::SurfaceKHR m_Surface;

	private:
		Window(const Window& y) = delete;
		Window operator=(const Window& y) = delete;
	};
}