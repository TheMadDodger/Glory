#include "VulkanDevice.h"
#include "VulkanGraphicsModule.h"

#include <Debug.h>

namespace Glory
{
	VulkanDevice::VulkanDevice(VulkanGraphicsModule* pModule, vk::PhysicalDevice physicalDevice):
		GraphicsDevice(pModule), m_VKDevice(physicalDevice), m_DidLastSupportCheckPass(false)
	{
	}

	VulkanDevice::~VulkanDevice()
	{
	}

	VulkanGraphicsModule* VulkanDevice::GraphicsModule()
	{
		return static_cast<VulkanGraphicsModule*>(m_pModule);
	}

	void VulkanDevice::LoadData()
	{
		// Find queue families
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties((VkPhysicalDevice)m_VKDevice, &queueFamilyCount, nullptr);
		m_AvailableQueueFamilies.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties((VkPhysicalDevice)m_VKDevice, &queueFamilyCount, m_AvailableQueueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : m_AvailableQueueFamilies)
		{
			if (m_GraphicsFamily.has_value() && m_PresentFamily.has_value()) break;
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR((VkPhysicalDevice)m_VKDevice, i, GraphicsModule()->GetCSurface(), &presentSupport);
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				m_GraphicsFamily = i;
			if (presentSupport)
				m_PresentFamily = i;
			++i;
		}

		// Get available extensions
		uint32_t deviceExtensionCount;
		vkEnumerateDeviceExtensionProperties((VkPhysicalDevice)m_VKDevice, nullptr, &deviceExtensionCount, nullptr);
		m_AvailableExtensions.resize(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties((VkPhysicalDevice)m_VKDevice, nullptr, &deviceExtensionCount, m_AvailableExtensions.data());

		// Get memory properties
		m_VKDevice.getMemoryProperties(&m_MemoryProperties);

		// Get device properties
		m_VKDevice.getProperties(&m_DeviceProperties);
	}

	void VulkanDevice::CheckSupport(std::vector<const char*> extensions)
	{
		m_DeviceExtensions = extensions;

		m_DidLastSupportCheckPass = false;

		if (!m_GraphicsFamily.has_value() || !m_PresentFamily.has_value()) return;

		std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

		for (const auto& extension : m_AvailableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		if (!requiredExtensions.empty())
		{
			Debug().LogError("Missing required device extensions!");
			return;
		}

		// Check swapchain support
		vk::SurfaceKHR surface = GraphicsModule()->GetSurface();
		vk::Result result = m_VKDevice.getSurfaceCapabilitiesKHR(surface, &m_SwapChainCapabilities);
		if (result != vk::Result::eSuccess)
		{
			Debug().LogFatalError("Vulkan: Failed to get surface capabilities: Vulkan Error Code: " + std::to_string((uint32_t)result));
			return;
		}
		uint32_t formatCount = 0;
		m_SwapChainFormats = m_VKDevice.getSurfaceFormatsKHR(surface);
		m_SwapChainPresentModes = m_VKDevice.getSurfacePresentModesKHR(surface);

		bool swapChainAdequite = !m_SwapChainFormats.empty() && !m_SwapChainPresentModes.empty();
		if (!swapChainAdequite) return;

		// Check feature support
		m_VKDevice.getFeatures(&m_Features);
		if (!m_Features.samplerAnisotropy) return;

		m_DidLastSupportCheckPass = true;
	}

	bool VulkanDevice::SupportCheckPassed() const
	{
		return m_DidLastSupportCheckPass;
	}

	void VulkanDevice::CreateLogicalDevice()
	{
		// Create logical device
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { m_GraphicsFamily.value(), m_PresentFamily.value() };

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
		const std::vector<const char*>& validationLayers = GraphicsModule()->GetValidationLayers();
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#else
		deviceCreateInfo.enabledLayerCount = 0;
#endif // _DEBUG

		vk::Result result = m_VKDevice.createDevice(&deviceCreateInfo, nullptr, &m_LogicalDevice);
		if (result != vk::Result::eSuccess)
		{
			Debug().LogFatalError("Vulkan: Failed to create logical device: Vulkan Error Code: " + std::to_string((uint32_t)result));
			return;
		}

		// Get the queue families from the device
		m_GraphicsQueue = m_LogicalDevice.getQueue(m_GraphicsFamily.value(), 0);
		m_PresentQueue = m_LogicalDevice.getQueue(m_PresentFamily.value(), 0);

		CreateGraphicsCommandPool();
	}

	uint32_t VulkanDevice::GetSupportedMemoryIndex(uint32_t typeFilter, vk::MemoryPropertyFlags propertyFlags)
	{
		for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; i++)
		{
			if (typeFilter & (1 << i) && (m_MemoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
				return i;
		}

		Debug().LogError("Vulkan: Failed to find suitable memory type.");
		return 0;
	}

	void VulkanDevice::CreateGraphicsCommandPool()
	{
		// Create command pool
		vk::CommandPoolCreateInfo commandPoolCreateInfo = vk::CommandPoolCreateInfo()
			.setQueueFamilyIndex(m_GraphicsFamily.value())
			.setFlags((vk::CommandPoolCreateFlags)0);

		m_GraphicsCommandPool = m_LogicalDevice.createCommandPool(commandPoolCreateInfo);
		if (m_GraphicsCommandPool == nullptr)
			throw std::runtime_error("failed to create command pool!");
	}

	vk::CommandPool VulkanDevice::CreateGraphicsCommandPool(vk::CommandPoolCreateFlags flags)
	{
		// Create command pool
		vk::CommandPoolCreateInfo commandPoolCreateInfo = vk::CommandPoolCreateInfo()
			.setQueueFamilyIndex(m_GraphicsFamily.value())
			.setFlags(flags);

		vk::CommandPool pool = m_LogicalDevice.createCommandPool(commandPoolCreateInfo);
		if (pool == nullptr)
			throw std::runtime_error("failed to create command pool!");

		m_GraphicsCommandPools.emplace(flags, pool);
		return pool;
	}

	vk::CommandPool VulkanDevice::GetGraphicsCommandPool()
	{
		return m_GraphicsCommandPool;
	}

	vk::CommandPool VulkanDevice::GetGraphicsCommandPool(vk::CommandPoolCreateFlags flags)
	{
		if (flags == (vk::CommandPoolCreateFlags)0) return GetGraphicsCommandPool();
		auto it = m_GraphicsCommandPools.find(flags);
		if (it == m_GraphicsCommandPools.end()) return CreateGraphicsCommandPool(flags);
		return m_GraphicsCommandPools[flags];
	}

	void VulkanDevice::BeginRenderPass(RenderPassHandle handle)
	{
	}

	void VulkanDevice::BeginPipeline(PipelineHandle handle)
	{
	}

	void VulkanDevice::EndRenderPass()
	{
	}

	void VulkanDevice::EndPipeline()
	{
	}

	void VulkanDevice::DrawMesh(MeshHandle handle)
	{
	}

	BufferHandle VulkanDevice::CreateBuffer(size_t bufferSize, BufferType type)
	{
		return BufferHandle();
	}

	void VulkanDevice::AssignBuffer(BufferHandle handle, const void* data)
	{
	}

	void VulkanDevice::AssignBuffer(BufferHandle handle, const void* data, uint32_t size)
	{
	}

	void VulkanDevice::AssignBuffer(BufferHandle handle, const void* data, uint32_t offset, uint32_t size)
	{
	}

	MeshHandle VulkanDevice::CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount, uint32_t indexCount, uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes)
	{
		return MeshHandle();
	}

	TextureHandle VulkanDevice::CreateTexture(TextureData* pTexture)
	{
		return TextureHandle();
	}

	TextureHandle VulkanDevice::CreateTexture(const TextureCreateInfo& textureInfo, const void* pixels)
	{
		return TextureHandle();
	}

	RenderTextureHandle VulkanDevice::CreateRenderTexture(RenderPassHandle renderPass, const RenderTextureCreateInfo& info)
	{
		return RenderTextureHandle();
	}

	RenderPassHandle VulkanDevice::CreateRenderPass(const RenderPassInfo& info)
	{
		return RenderPassHandle();
	}

	ShaderHandle VulkanDevice::CreateShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function)
	{
		return ShaderHandle();
	}

	PipelineHandle VulkanDevice::CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline)
	{
		return PipelineHandle();
	}

	void VulkanDevice::FreeBuffer(BufferHandle& handle)
	{
	}

	void VulkanDevice::FreeMesh(MeshHandle& handle)
	{
	}

	void VulkanDevice::FreeTexture(TextureHandle& handle)
	{
	}

	void VulkanDevice::FreeRenderTexture(RenderTextureHandle& handle)
	{
	}

	void VulkanDevice::FreeRenderPass(RenderPassHandle& handle)
	{
	}
}
