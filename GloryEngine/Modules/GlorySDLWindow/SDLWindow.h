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
		GLORY_API virtual void SetCursorPosition(int x, int y) override;

		GLORY_API virtual void SetWindowTitle(const char* title) override;

		GLORY_API virtual void GetWindowBordersSize(int* top, int* left, int* bottom, int* right) override;

		virtual void SetSplashScreen(const char* data, size_t size) override;
		virtual void SetSplashScreen(const std::filesystem::path& path) override;

		GLORY_API SDL_Window* GetSDLWindow();
		GLORY_API SDL_GLContext GetSDLGLConext() const;

		GLORY_API bool PollEvent(SDL_Event* event);
		GLORY_API bool HandleInputEvents(SDL_Event& event);
		GLORY_API void HandleWindowFocusEvents(SDL_WindowEvent& event);
		GLORY_API void HandleWindowSizeEvents(SDL_WindowEvent& event);

	private:
		SDLWindow(const WindowCreateInfo& createInfo);
		virtual ~SDLWindow();

		void HandleAllEvents(SDL_Event& event);
		virtual void UpdateCursorShow() override;

	private:
		GLORY_API virtual void Open() override;
		GLORY_API virtual void Close() override;
		GLORY_API virtual void PollEvents() override;
		GLORY_API virtual void GetVulkanRequiredExtensions(std::vector<const char*>& extensions) override;

	private:
		friend class SDLWindowModule;
		SDL_Window* m_pWindow;
		SDL_Surface* m_pWindowSurface;
		SDL_Surface* m_pSplashScreen;
		SDL_GLContext m_GLSDLContext;
	};
}