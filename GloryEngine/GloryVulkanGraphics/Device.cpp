#include "Device.h"
#include <set>
#include <iostream>
#include "VulkanExceptions.h"
#include <Window.h>

namespace Glory
{
    const SwapChainSupportDetails& Device::GetSwapChainSupportDetails() const
    {
        return m_SwapChainSupportDetails;
    }

    const QueueFamilyIndices& Device::GetQueueFamilyIndices() const
    {
        return m_QueueFamilyIndices;
    }

    const LogicalDeviceData& Device::GetLogicalDeviceData() const
    {
        return m_LogicalDeviceData;
    }

    void Device::CreateLogicalDevice(VulkanGraphicsModule* pGraphicsModule)
    {
        // Create logical device
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { m_QueueFamilyIndices.GraphicsFamily.value(), m_QueueFamilyIndices.PresentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            vk::DeviceQueueCreateInfo queueCreateInfo = vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(queueFamily)
                .setQueueCount(1)
                .setPQueuePriorities(&queuePriority);
            queueCreateInfos.push_back(queueCreateInfo);
        }

        vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();
        vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo()
            .setPQueueCreateInfos(queueCreateInfos.data())
            .setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
            .setPEnabledFeatures(&deviceFeatures)
            .setEnabledExtensionCount(static_cast<uint32_t>(m_DeviceExtensions.size()))
            .setPpEnabledExtensionNames(m_DeviceExtensions.data());
        deviceFeatures.samplerAnisotropy = VK_TRUE;

#if defined(_DEBUG)
        const std::vector<const char*>& validationLayers = pGraphicsModule->GetValidationLayers();
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#else
        deviceCreateInfo.enabledLayerCount = 0;
#endif // _DEBUG

        vk::Result result = m_PhysicalDevice.createDevice(&deviceCreateInfo, nullptr, &m_LogicalDeviceData.LogicalDevice);
        if (result != vk::Result::eSuccess)
            throw new VulkanException(result);

        // Get the queue families from the device
        m_LogicalDeviceData.GraphicsQueue = m_LogicalDeviceData.LogicalDevice.getQueue(m_QueueFamilyIndices.GraphicsFamily.value(), 0);
        m_LogicalDeviceData.PresentQueue = m_LogicalDeviceData.LogicalDevice.getQueue(m_QueueFamilyIndices.PresentFamily.value(), 0);

        CreateGraphicsCommandPool();
    }

    vk::PhysicalDevice Device::GetPhysicalDevice()
    {
        return m_PhysicalDevice;
    }

    uint32_t Device::GetSupportedMemoryIndex(uint32_t typeFilter, vk::MemoryPropertyFlags propertyFlags)
    {
        for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; i++)
        {
            if (typeFilter & (1 << i) && (m_MemoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
            {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type!");
    }

    vk::CommandPool Device::GetGraphicsCommandPool()
    {
        return m_GraphicsCommandPool;
    }

    const vk::PhysicalDeviceProperties& Device::GetDeviceProperties() const
    {
        return m_DeviceProperties;
    }

    vk::Format Device::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
    {
        for (size_t i = 0; i < candidates.size(); i++)
        {
            vk::Format format = candidates[i];
            vk::FormatProperties props;
            m_PhysicalDevice.getFormatProperties(format, &props);

            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
                return format;

            else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
                return format;
        }

        throw std::runtime_error("Failed to find supported format!");
    }

    vk::Format Device::FindDepthFormat()
    {
        vk::Format supportedDepthFormat =
            FindSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
                vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

        return supportedDepthFormat;
    }

    bool Device::HasStencilComponent(vk::Format format)
    {
        return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
    }

    Device::Device(vk::PhysicalDevice physicalDevice) : m_PhysicalDevice(physicalDevice), m_cPhysicalDevice((VkPhysicalDevice)physicalDevice),
        m_QueueFamilyIndices(), m_AvailableQueueFamilies(std::vector<VkQueueFamilyProperties>()), m_AvailableExtensions(std::vector<VkExtensionProperties>()),
        m_DidLastSupportCheckPass(false), m_SwapChainSupportDetails(), m_LogicalDeviceData(), m_DeviceExtensions(std::vector<const char*>()) {}

	Device::~Device()
	{
        if (m_LogicalDeviceData.LogicalDevice) m_LogicalDeviceData.LogicalDevice.destroy();

        m_AvailableQueueFamilies.clear();
        m_AvailableExtensions.clear();
        m_SwapChainSupportDetails.Formats.clear();
        m_SwapChainSupportDetails.PresentModes.clear();
        m_DeviceExtensions.clear();

        if (m_GraphicsCommandPool)m_LogicalDeviceData.LogicalDevice.destroyCommandPool(m_GraphicsCommandPool);
	}

	void Device::LoadData(VulkanGraphicsModule* pGraphicsModule)
	{
        // Find queue families
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_cPhysicalDevice, &queueFamilyCount, nullptr);
        m_AvailableQueueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_cPhysicalDevice, &queueFamilyCount, m_AvailableQueueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : m_AvailableQueueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                m_QueueFamilyIndices.GraphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_cPhysicalDevice, i, pGraphicsModule->GetCSurface(), &presentSupport);
            if (presentSupport) m_QueueFamilyIndices.PresentFamily = i;

            if (m_QueueFamilyIndices.IsComplete()) break;
            i++;
        }

        // Get available extensions
        uint32_t deviceExtensionCount;
        vkEnumerateDeviceExtensionProperties(m_cPhysicalDevice, nullptr, &deviceExtensionCount, nullptr);
        m_AvailableExtensions.resize(deviceExtensionCount);
        vkEnumerateDeviceExtensionProperties(m_cPhysicalDevice, nullptr, &deviceExtensionCount, m_AvailableExtensions.data());

        // Get memory properties
        m_PhysicalDevice.getMemoryProperties(&m_MemoryProperties);

        // Get device properties
        m_PhysicalDevice.getProperties(&m_DeviceProperties);
	}

    bool Device::CheckSupport(VulkanGraphicsModule* pGraphicsModule, std::vector<const char*> extensions)
    {
        m_DeviceExtensions = extensions;

        m_DidLastSupportCheckPass = false;

        if (!m_QueueFamilyIndices.IsComplete()) return false;
        
        std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());
        
        for (const auto& extension : m_AvailableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }
        
        if (!requiredExtensions.empty())
        {
            std::cerr << "Missing required device extensions!" << std::endl;
            return false;
        }
        
        // Check swapchain support
        vk::SurfaceKHR surface = pGraphicsModule->GetSurface();
        m_PhysicalDevice.getSurfaceCapabilitiesKHR(surface, &m_SwapChainSupportDetails.Capabilities);
        uint32_t formatCount = 0;
        m_SwapChainSupportDetails.Formats = m_PhysicalDevice.getSurfaceFormatsKHR(surface);
        m_SwapChainSupportDetails.PresentModes = m_PhysicalDevice.getSurfacePresentModesKHR(surface);
        
        bool swapChainAdequite = !m_SwapChainSupportDetails.Formats.empty() && !m_SwapChainSupportDetails.PresentModes.empty();
        if (!swapChainAdequite) return false;

        m_DidLastSupportCheckPass = true;

        // Check feature support
        m_PhysicalDevice.getFeatures(&m_Features);
        if (!m_Features.samplerAnisotropy) return false;

        return true;
    }

    void Device::CreateGraphicsCommandPool()
    {
        // Create command pool
        vk::CommandPoolCreateInfo commandPoolCreateInfo = vk::CommandPoolCreateInfo()
            .setQueueFamilyIndex(m_QueueFamilyIndices.GraphicsFamily.value())
            .setFlags((vk::CommandPoolCreateFlags)0);

        m_GraphicsCommandPool = m_LogicalDeviceData.LogicalDevice.createCommandPool(commandPoolCreateInfo);
        if (m_GraphicsCommandPool == nullptr)
            throw std::runtime_error("failed to create command pool!");
    }
}