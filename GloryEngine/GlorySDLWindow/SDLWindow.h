#pragma once
#include <Window.h>
#include <SDL2/SDL.h>

namespace Glory
{
	class SDLWindow : public Window
	{
	public:
		virtual void GetVulkanSurface(void* instance, void* surface) override;
		virtual void GetDrawableSize(int* width, int* height) override;
		virtual void GetWindowSize(int* width, int* height) override;
		virtual void GetWindowPosition(int* x, int* y) override;
		SDL_Window* GetSDLWindow();

	private:
		SDLWindow(const WindowCreateInfo& createInfo);
		virtual ~SDLWindow();

	private:
		virtual void Open() override;
		virtual void Close() override;
		virtual void PollEvents() override;
		virtual void GetVulkanRequiredExtensions(std::vector<const char*>& extensions) override;

	private:
		friend class SDLWindowModule;
		SDL_Window* m_pWindow;
	};
}