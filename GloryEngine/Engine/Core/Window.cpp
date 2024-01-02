#include "Window.h"
#include "WindowModule.h"
#include "InputModule.h"
#include "Engine.h"

namespace Glory
{
	Window::Window(const WindowCreateInfo& createInfo) :
		m_WindowName(createInfo.WindowName), m_Width(createInfo.Width),
		m_Height(createInfo.Height), m_WindowFlags(createInfo.WindowFlags),
		m_pWindowManager(createInfo.pWindowManager)
	{
	}

	Window::~Window()
	{
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
}