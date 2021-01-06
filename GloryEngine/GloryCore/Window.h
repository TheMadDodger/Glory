#pragma once
#include "Object.h"
#include <string>
#include <vector>

namespace Glory
{
	struct WindowCreateInfo
	{
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

	protected:
		Window(const WindowCreateInfo& createInfo);
		virtual ~Window();

		virtual void Open() = 0;
		virtual void Close() = 0;
		virtual void PollEvents() = 0;

	protected:
		std::string m_WindowName;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_WindowFlags;

	private:
		friend class WindowModule;

	private:
		Window(const Window& y) = delete;
		Window operator=(const Window& y) = delete;
	};
}