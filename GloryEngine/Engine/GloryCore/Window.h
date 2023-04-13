#pragma once
#include "Object.h"
#include <string>
#include <vector>
#include "Input.h"

namespace Glory
{
	class WindowModule;

	struct WindowCreateInfo
	{
		WindowModule* pWindowManager;
		std::string WindowName;
		uint32_t Width;
		uint32_t Height;
		uint32_t WindowFlags;
	};

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

		virtual void SetWindowTitle(const char* title) = 0;

		virtual void GetWindowBordersSize(int* top, int* left, int* bottom, int* right) = 0;

	protected:
		Window(const WindowCreateInfo& createInfo);
		virtual ~Window();

		virtual void Open() = 0;
		virtual void Close() = 0;
		virtual void PollEvents() = 0;

		bool ForwardInputEvent(InputEvent& input);

	protected:
		std::string m_WindowName;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_WindowFlags;
		WindowModule* m_pWindowManager;

	private:
		friend class WindowModule;

	private:
		Window(const Window& y) = delete;
		Window operator=(const Window& y) = delete;
	};
}