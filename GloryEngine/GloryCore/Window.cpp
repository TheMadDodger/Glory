#include "Window.h"
#include "CoreExceptions.h"

namespace Glory
{
	Window::Window(const WindowCreateInfo& createInfo) :
		m_WindowName(createInfo.WindowName), m_Width(createInfo.Width),
		m_Height(createInfo.Height), m_WindowFlags(createInfo.WindowFlags)
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
//
//    std::vector<char> Window::ReadFile(const std::string& filename)
//    {
//        std::ifstream file(filename, std::ios::ate | std::ios::binary);
//
//        if (!file.is_open())
//        {
//            throw std::runtime_error("failed to open file!");
//        }
//
//        size_t fileSize = (size_t)file.tellg();
//        std::vector<char> buffer(fileSize);
//        file.seekg(0);
//        file.read(buffer.data(), fileSize);
//        file.close();
//        return buffer;
//    }
}