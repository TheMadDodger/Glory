#pragma once
#include <Window.h>
#include <SDL2/SDL.h>

namespace Glory
{
	class SDLWindow : public Window
	{
	public:
		void GetVulkanSurface(void* instance, void* surface) override;
		void GetDrawableSize(int* width, int* height) override;

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