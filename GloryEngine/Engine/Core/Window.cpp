#include "Window.h"
#include "WindowModule.h"
#include "InputModule.h"
#include "Engine.h"
#include "WindowData.h"
#include "IWindowInputOverrideHandler.h"

namespace Glory
{
	Window::Window(const WindowCreateInfo& createInfo, WindowModule* pWindowManager) :
		m_WindowName(createInfo.WindowName), m_Width(createInfo.Width),
		m_Height(createInfo.Height), m_WindowFlags(createInfo.WindowFlags),
		m_pWindowManager(pWindowManager), m_ShowCursor(true),
		m_ForceShowCursor(false), m_ForceUnlockCursor(false), m_GrabInput(false),
		m_ForceUngrabInput(false), m_HasFocus(false), m_IsShown(false),
		m_Fullscreen(createInfo.Fullscreen), m_Maximized(createInfo.Maximize)
	{
	}

	Window::~Window()
	{
		m_pInputOverrideHandlers.clear();
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

	void Window::AddInputOverrideHandler(IWindowInputOverrideHandler* handler)
	{
		m_pInputOverrideHandlers.push_back(handler);
	}

	void Window::RemoveInputOverrideHandler(IWindowInputOverrideHandler* handler)
	{
		auto iter = std::find(m_pInputOverrideHandlers.begin(), m_pInputOverrideHandlers.end(), handler);
		if (iter == m_pInputOverrideHandlers.end()) return;
		m_pInputOverrideHandlers.erase(iter);
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
		for (size_t i = 0; i < m_pInputOverrideHandlers.size(); ++i)
		{
			if (!m_pInputOverrideHandlers[i]->OnOverrideInputEvent(input)) continue;
			return true;
		}

		InputModule* pInput = m_pWindowManager->GetEngine()->GetMainModule<InputModule>();
		return pInput->OnInput(input);
	}

	void Window::ForwardCursorEvent(CursorEvent& input)
	{
		InputModule* pInput = m_pWindowManager->GetEngine()->GetMainModule<InputModule>();
		pInput->OnCursor(input);
	}

	bool Window::ForwardTextEvent(TextEvent& text)
	{
		for (size_t i = 0; i < m_pInputOverrideHandlers.size(); ++i)
		{
			if (!m_pInputOverrideHandlers[i]->OnOverrideTextEvent(text)) continue;
			return true;
		}

		InputModule* pInput = m_pWindowManager->GetEngine()->GetMainModule<InputModule>();
		return pInput->OnText(text);
	}
}