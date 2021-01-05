#pragma once
#include "Object.h"
#include <string>

namespace Glory
{
	//struct SwapChainSupportDetails
	//{
	//	vk::SurfaceCapabilitiesKHR Capabilities;
	//	std::vector<vk::SurfaceFormatKHR> Formats;
	//	std::vector<vk::PresentModeKHR> PresentModes;
	//};

	struct WindowCreateInfo
	{
		std::string WindowName;
		uint32_t Width;
		uint32_t Height;
	};

	class Window : public Object
	{
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

	private:
		friend class WindowModule;

		//unsigned m_ExtensionCount;
		//std::vector<const char*> m_Extensions;
		//std::vector<const char*> m_Layers;
		//vk::Instance m_Instance;
		//VkSurfaceKHR m_cSurface;
		//vk::SurfaceKHR m_Surface;
		//vk::DebugUtilsMessengerEXT m_DebugMessenger;
		//vk::PhysicalDevice m_PhysicalDevice;
		//std::optional<uint32_t> graphicsFamily;
		//std::optional<uint32_t> presentFamily;
		//vk::Device m_Device;
		//vk::Queue m_GraphicsQueue;
		//vk::Queue m_PresentQueue;
		//vk::SwapchainKHR m_SwapChain;
		//std::vector<vk::Image> m_SwapChainImages;
		//vk::Format m_SwapChainImageFormat;
		//vk::Extent2D m_SwapChainExtent;
		//std::vector<vk::ImageView> m_SwapChainImageViews;
		//vk::RenderPass m_RenderPass;
		//vk::PipelineLayout m_PipelineLayout;
		//vk::Pipeline m_GraphicsPipeline;
		//std::vector<vk::Framebuffer> m_SwapChainFramebuffers;
		//vk::CommandPool m_CommandPool;
		//std::vector<vk::CommandBuffer> m_CommandBuffers;
		//vk::Semaphore m_ImageAvailableSemaphore;
		//vk::Semaphore m_RenderFinishedSemaphore;

		//static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		//	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		//	VkDebugUtilsMessageTypeFlagsEXT messageType,
		//	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		//	void* pUserData);
		//
		//static std::vector<char> ReadFile(const std::string& filename);

	private:
		Window(const Window& y) = delete;
		Window operator=(const Window& y) = delete;
	};
}