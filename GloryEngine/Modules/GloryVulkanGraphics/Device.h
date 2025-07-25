#pragma once
#include "QueueFamilyIndices.h"
#include "VulkanGraphicsModule.h"

#include <vulkan/vulkan.hpp>

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
		GLORY_API const SwapChainSupportDetails& GetSwapChainSupportDetails() const;
		GLORY_API const QueueFamilyIndices& GetQueueFamilyIndices() const;
		GLORY_API const LogicalDeviceData& GetLogicalDeviceData() const;
		GLORY_API void CreateLogicalDevice(VulkanGraphicsModule* pGraphicsModule);
		GLORY_API vk::PhysicalDevice GetPhysicalDevice();
		GLORY_API uint32_t GetSupportedMemoryIndex(uint32_t typeFilter, vk::MemoryPropertyFlags propertyFlags);

		GLORY_API vk::CommandPool GetGraphicsCommandPool();
		GLORY_API vk::CommandPool GetGraphicsCommandPool(vk::CommandPoolCreateFlags flags);
		GLORY_API const vk::PhysicalDeviceProperties& GetDeviceProperties() const;
		GLORY_API vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		GLORY_API vk::Format FindDepthFormat();
		GLORY_API bool HasStencilComponent(vk::Format format);

	private:
		Device(vk::PhysicalDevice physicalDevice);
		virtual ~Device();

		void LoadData(VulkanGraphicsModule* pGraphicsModule);
		bool CheckSupport(VulkanGraphicsModule* pGraphicsModule, std::vector<const char*> extensions);

		void CreateGraphicsCommandPool();
		vk::CommandPool CreateGraphicsCommandPool(vk::CommandPoolCreateFlags flags);

	private:
		friend class VulkanDeviceManager;
		vk::PhysicalDevice m_PhysicalDevice;
		vk::PhysicalDeviceMemoryProperties m_MemoryProperties;
		VkPhysicalDevice m_cPhysicalDevice;
		std::vector<VkQueueFamilyProperties> m_AvailableQueueFamilies;
		std::vector<VkExtensionProperties> m_AvailableExtensions;
		QueueFamilyIndices m_QueueFamilyIndices;
		bool m_DidLastSupportCheckPass;
		SwapChainSupportDetails m_SwapChainSupportDetails;
		LogicalDeviceData m_LogicalDeviceData;
		std::vector<const char*> m_DeviceExtensions;
		vk::CommandPool m_GraphicsCommandPool;
		std::map<vk::CommandPoolCreateFlags, vk::CommandPool> m_GraphicsCommandPools;
		vk::PhysicalDeviceFeatures m_Features;
		vk::PhysicalDeviceProperties m_DeviceProperties;
	};
}
