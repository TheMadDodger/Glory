#include "Window.h"
#include "WindowModule.h"
#include "InputModule.h"
#include "Engine.h"

namespace Glory
{
	Window::Window(const WindowCreateInfo& createInfo) :
		m_WindowName(createInfo.WindowName), m_Width(createInfo.Width),
		m_Height(createInfo.Height), m_WindowFlags(createInfo.WindowFlags),
		m_pWindowManager(createInfo.pWindowManager), m_ShowCursor(true),
		m_ForceShowCursor(false), m_ForceUnlockCursor(false), m_GrabInput(false),
		m_ForceUngrabInput(false), m_HasFocus(false), m_IsShown(false),
		m_Fullscreen(createInfo.Fullscreen), m_Maximized(createInfo.Maximize)
	{
	}

	Window::~Window()
	{
	}

	void Window::ShowCursor(bool show)
	{
		if (m_ShowCursor == show) return;
		m_ShowCursor = show;
		UpdateCursorShow();
	}

	void Window::ForceShowCursor(bool show)
	{
		if (m_ForceShowCursor == show) return;
		m_ForceShowCursor = show;
		UpdateCursorShow();
	}

	void Window::ForceUnlockCursor(bool unlock)
	{
		m_ForceUnlockCursor = unlock;
	}

	bool Window::IsCursorShown() const
	{
		return m_ForceShowCursor || m_ShowCursor;
	}

	void Window::GrabInput(bool grab)
	{
		if (m_GrabInput == grab) return;
		m_GrabInput = grab;
		UpdateGrabInput();
	}

	bool Window::IsGrabInput()
	{
		return m_GrabInput && !m_ForceUngrabInput;
	}
	
	void Window::ForceUngrabInput(bool ungrab)
	{
		if (m_ForceUngrabInput == ungrab) return;
		m_ForceUngrabInput = ungrab;
		UpdateGrabInput();
	}

	bool Window::HasFocus() const
	{
		return m_HasFocus;
	}

	bool Window::IsShown() const
	{
		return m_IsShown;
	}

	void Window::GetVulkanRequiredExtensions(std::vector<const char*>&) {}

	void Window::GetVulkanSurface(void* instance, void* surface) {}

	void Window::GetDrawableSize(int* width, int* height)
	{
		GetWindowSize(width, height);
	}

	void Window::GetWindowSize(int* width, int* height)
	{
		*width = m_Width;
		*height = m_Height;
	}

	bool Window::ForwardInputEvent(InputEvent& input)
	{
		InputModule* pInput = m_pWindowManager->GetEngine()->GetMainModule<InputModule>();
		return pInput->OnInput(input);
	}

	void Window::ForwardCursorEvent(CursorEvent& input)
	{
		InputModule* pInput = m_pWindowManager->GetEngine()->GetMainModule<InputModule>();
		return pInput->OnCursor(input);
	}
}