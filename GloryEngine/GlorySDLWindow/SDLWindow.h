#pragma once
#include <Window.h>
#include <SDL2/SDL.h>

namespace Glory
{
	class SDLWindow : public Window
	{
	private:
		SDLWindow(const WindowCreateInfo& createInfo);
		virtual ~SDLWindow();

	private:
		virtual void Open() override;
		virtual void Close() override;
		virtual void PollEvents() override;

	private:
		friend class SDLWindowModule;
		SDL_Window* m_pWindow;
	};
}