#pragma once
#include <vulkan/vulkan.hpp>
#include "QueueFamilyIndices.h"
#include "VulkanGraphicsModule.h"

namespace Glory
{
	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR Capabilities;
		std::vector<vk::SurfaceFormatKHR> Formats;
		std::vector<vk::PresentModeKHR> PresentModes;
	};

	struct LogicalDeviceData
	{
		vk::Device LogicalDevice;
		vk::Queue GraphicsQueue;
		vk::Queue PresentQueue;
	};

	class Window;

	class Device
	{
	public:
		const SwapChainSupportDetails& GetSwapChainSupportDetails() const;
		const QueueFamilyIndices& GetQueueFamilyIndices() const;
		const LogicalDeviceData& GetLogicalDeviceData() const;
		void CreateLogicalDevice(VulkanGraphicsModule* pGraphicsModule);
		vk::PhysicalDevice GetPhysicalDevice();

	private:
		Device(vk::PhysicalDevice physicalDevice);
		virtual ~Device();

		void LoadData(VulkanGraphicsModule* pGraphicsModule);
		bool CheckSupport(VulkanGraphicsModule* pGraphicsModule, std::vector<const char*> extensions);

	private:
		friend class VulkanDeviceManager;
		vk::PhysicalDevice m_PhysicalDevice;
		VkPhysicalDevice m_cPhysicalDevice;
		std::vector<VkQueueFamilyProperties> m_AvailableQueueFamilies;
		std::vector<VkExtensionProperties> m_AvailableExtensions;
		QueueFamilyIndices m_QueueFamilyIndices;
		bool m_DidLastSupportCheckPass;
		SwapChainSupportDetails m_SwapChainSupportDetails;
		LogicalDeviceData m_LogicalDeviceData;
		std::vector<const char*> m_DeviceExtensions;
	};
}
