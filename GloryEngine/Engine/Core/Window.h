#pragma once
#include "Object.h"
#include "Input.h"

#include <string>
#include <vector>

namespace std::filesystem
{
	class path;
}

namespace Glory
{
	class IWindowInputOverrideHandler;
	class WindowModule;
	struct WindowCreateInfo;

	class Window : public Object
	{
	public:
		virtual void GetVulkanRequiredExtensions(std::vector<const char*>& extensions);
		virtual void GetVulkanSurface(void* instance, void* surface);
		virtual void GetDrawableSize(int* width, int* height);
		virtual void GetWindowSize(int* width, int* height);
		virtual void GetWindowPosition(int* x, int* y) = 0;
		virtual void SetupForOpenGL() = 0;
		virtual void CleanupOpenGL() = 0;
		virtual void GLSwapWindow() = 0;
		virtual void MakeGLContextCurrent() = 0;
		virtual void Resize(int width, int height) = 0;
		virtual void GetPosition(int* x, int* y) = 0;
		virtual void SetPosition(int x, int y) = 0;
		virtual void SetCursorPosition(int x, int y) = 0;
		virtual void SetFullscreen(bool fullscreen, bool borderless) = 0;
		virtual void Maximize() = 0;

		virtual void SetWindowTitle(const char* title) = 0;

		virtual void GetWindowBordersSize(int* top, int* left, int* bottom, int* right) = 0;

		virtual void SetSplashScreen(const char* data, size_t size) = 0;
		virtual void SetSplashScreen(const std::filesystem::path& path) = 0;
		virtual std::string GetClipboardText() = 0;
		virtual void StartTextInput() = 0;
		virtual void StopTextInput() = 0;

		void ShowCursor(bool shown);
		void ForceShowCursor(bool show);
		void ForceUnlockCursor(bool unlock);
		bool IsCursorShown() const;
		void GrabInput(bool grab);
		bool IsGrabInput();
		void ForceUngrabInput(bool ungrab);
		bool HasFocus() const;
		bool IsShown() const;

		void AddInputOverrideHandler(IWindowInputOverrideHandler* handler);
		void RemoveInputOverrideHandler(IWindowInputOverrideHandler* handler);

	protected:
		Window(const WindowCreateInfo& createInfo);
		virtual ~Window();

		virtual void Open() = 0;
		virtual void Close() = 0;
		virtual void PollEvents() = 0;
		virtual void UpdateCursorShow() = 0;
		virtual void UpdateGrabInput() = 0;

		bool ForwardInputEvent(InputEvent& input);
		void ForwardCursorEvent(CursorEvent& input);
		bool ForwardTextEvent(TextEvent& text);

	protected:
		std::string m_WindowName;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_WindowFlags;
		WindowModule* m_pWindowManager;
		bool m_ShowCursor;
		bool m_ForceShowCursor;
		bool m_ForceUnlockCursor;
		bool m_GrabInput;
		bool m_ForceUngrabInput;

		bool m_HasFocus;
		bool m_IsShown;

		bool m_Fullscreen;
		bool m_Maximized;

		std::vector<IWindowInputOverrideHandler*> m_pInputOverrideHandlers;

	private:
		friend class WindowModule;

	private:
		Window(const Window& y) = delete;
		Window operator=(const Window& y) = delete;
	};
}