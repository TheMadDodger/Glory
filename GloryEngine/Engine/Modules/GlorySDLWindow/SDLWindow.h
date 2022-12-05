#pragma once
#include <Window.h>
#include <SDL2/SDL.h>
#include <Glory.h>

namespace Glory
{
	class SDLWindow : public Window
	{
	public:
		GLORY_API virtual void GetVulkanSurface(void* instance, void* surface) override;
		GLORY_API virtual void GetDrawableSize(int* width, int* height) override;
		GLORY_API virtual void GetWindowSize(int* width, int* height) override;
		GLORY_API virtual void GetWindowPosition(int* x, int* y) override;
		GLORY_API virtual void SetupForOpenGL() override;
		GLORY_API virtual void CleanupOpenGL() override;
		GLORY_API virtual void GLSwapWindow() override;
		GLORY_API virtual void MakeGLContextCurrent() override;
		GLORY_API virtual void Resize(int width, int height) override;
		GLORY_API virtual void GetPosition(int* x, int* y) override;
		GLORY_API virtual void SetPosition(int x, int y) override;

		GLORY_API SDL_Window* GetSDLWindow();
		GLORY_API SDL_GLContext GetSDLGLConext() const;

	private:
		SDLWindow(const WindowCreateInfo& createInfo);
		virtual ~SDLWindow();

	private:
		GLORY_API virtual void Open() override;
		GLORY_API virtual void Close() override;
		GLORY_API virtual void PollEvents() override;
		GLORY_API virtual void GetVulkanRequiredExtensions(std::vector<const char*>& extensions) override;

	private:
		friend class SDLWindowModule;
		SDL_Window* m_pWindow;
		SDL_GLContext m_GLSDLContext;
	};
}