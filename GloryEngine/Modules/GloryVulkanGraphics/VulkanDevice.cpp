#include "VulkanDevice.h"
#include "VulkanGraphicsModule.h"
#include "VulkanStructsConverter.h"

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
		BufferHandle handle;
		VK_Buffer& buffer = m_Buffers.Emplace(handle, VK_Buffer());
		buffer.m_Size = bufferSize;

		vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo();
		bufferInfo.size = (vk::DeviceSize)buffer.m_Size;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;

		switch (type)
		{
		case Glory::BT_TransferRead:
			bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
			break;
		case Glory::BT_TransferWrite:
			bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
			break;
		case Glory::BT_Vertex:
			bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
			break;
		case Glory::BT_Index:
			bufferInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer;
			break;
		default:
			break;
		}

		vk::Result result = m_LogicalDevice.createBuffer(&bufferInfo, nullptr, &buffer.m_VKBuffer);
		if (result != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateBuffer: Failed to create buffer.");
			m_Buffers.Erase(handle);
			return NULL;
		}

		vk::MemoryRequirements memRequirements;
		m_LogicalDevice.getBufferMemoryRequirements(buffer.m_VKBuffer, &memRequirements);

		uint32_t typeFilter = memRequirements.memoryTypeBits;
		vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;//(vk::MemoryPropertyFlagBits)m_MemoryFlags;
		uint32_t memoryIndex = GetSupportedMemoryIndex(typeFilter, properties);

		// Allocate device memory
		vk::MemoryAllocateInfo allocateInfo = vk::MemoryAllocateInfo();
		allocateInfo.allocationSize = memRequirements.size;
		allocateInfo.memoryTypeIndex = memoryIndex;

		result = m_LogicalDevice.allocateMemory(&allocateInfo, nullptr, &buffer.m_VKMemory);
		if (result != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateBuffer: Failed to create buffer memory.");
			m_Buffers.Erase(handle);
			return NULL;
		}

		m_LogicalDevice.bindBufferMemory(buffer.m_VKBuffer, buffer.m_VKMemory, 0);

		std::stringstream str;
		str << "VulkanDevice: Buffer " << handle << " created with size " << bufferSize << ".";
		Debug().LogInfo(str.str());

		return handle;
	}

	void VulkanDevice::AssignBuffer(BufferHandle handle, const void* data)
	{
		if (!data) return;

		VK_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("VulkanDevice::FreeBuffer: Invalid buffer handle.");
			return;
		}

		void* dstData;
		vk::Result result = m_LogicalDevice.mapMemory(buffer->m_VKMemory, (vk::DeviceSize)0, (vk::DeviceSize)buffer->m_Size, (vk::MemoryMapFlags)0, &dstData);
		if (result != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateBuffer: Failed to map buffer memory.");
			return;
		}
		memcpy(dstData, data, buffer->m_Size);
		m_LogicalDevice.unmapMemory(buffer->m_VKMemory);
	}

	void VulkanDevice::AssignBuffer(BufferHandle handle, const void* data, uint32_t size)
	{
		if (!data) return;

		VK_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("VulkanDevice::FreeBuffer: Invalid buffer handle.");
			return;
		}

		void* dstData;
		vk::Result result = m_LogicalDevice.mapMemory(buffer->m_VKMemory, (vk::DeviceSize)0, (vk::DeviceSize)size, (vk::MemoryMapFlags)0, &dstData);
		if (result != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateBuffer: Failed to map buffer memory.");
			return;
		}
		memcpy(dstData, data, size);
		m_LogicalDevice.unmapMemory(buffer->m_VKMemory);
	}

	void VulkanDevice::AssignBuffer(BufferHandle handle, const void* data, uint32_t offset, uint32_t size)
	{
		if (!data) return;

		VK_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("VulkanDevice::FreeBuffer: Invalid buffer handle.");
			return;
		}

		void* dstData;
		vk::Result result = m_LogicalDevice.mapMemory(buffer->m_VKMemory, (vk::DeviceSize)offset, (vk::DeviceSize)size, (vk::MemoryMapFlags)0, &dstData);
		if (result != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateBuffer: Failed to map buffer memory.");
			return;
		}
		memcpy(dstData, data, size);
		m_LogicalDevice.unmapMemory(buffer->m_VKMemory);
	}

	vk::Format GetFormat(const AttributeType& atributeType)
	{
		vk::Format format = vk::Format::eUndefined;

		switch (atributeType)
		{
		case AttributeType::Float:
			format = vk::Format::eR32Sfloat;
			break;
		case AttributeType::Float2:
			format = vk::Format::eR32G32Sfloat;
			break;
		case AttributeType::Float3:
			format = vk::Format::eR32G32B32Sfloat;
			break;
		case AttributeType::Float4:
			format = vk::Format::eR32G32B32A32Sfloat;
			break;
		case AttributeType::UINT:
			format = vk::Format::eR32Uint;
			break;
		case AttributeType::UINT2:
			format = vk::Format::eR32G32Uint;
			break;
		case AttributeType::UINT3:
			format = vk::Format::eR32G32B32Uint;
			break;
		case AttributeType::UINT4:
			format = vk::Format::eR32G32B32A32Uint;
			break;
		case AttributeType::SINT:
			format = vk::Format::eR32Sint;
			break;
		case AttributeType::SINT2:
			format = vk::Format::eR32G32Sint;
			break;
		case AttributeType::SINT3:
			format = vk::Format::eR32G32B32Sint;
			break;
		case AttributeType::SINT4:
			format = vk::Format::eR32G32B32A32Sint;
			break;
		}

		return format;
	}

	void GetNextOffset(const AttributeType& atributeType, uint32_t& offest)
	{
		switch (atributeType)
		{
		case AttributeType::Float:
			offest += sizeof(float);
			break;
		case AttributeType::Float2:
			offest += (sizeof(float)*2);
			break;
		case AttributeType::Float3:
			offest += (sizeof(float)*3);
			break;
		case AttributeType::Float4:
			offest += (sizeof(float)*4);
			break;
		case AttributeType::UINT:
			offest += (sizeof(uint32_t));
			break;
		case AttributeType::UINT2:
			offest += (sizeof(uint32_t)*2);
			break;
		case AttributeType::UINT3:
			offest += (sizeof(uint32_t)*3);
			break;
		case AttributeType::UINT4:
			offest += (sizeof(uint32_t)*4);
			break;
		case AttributeType::SINT:
			offest += (sizeof(int32_t));
			break;
		case AttributeType::SINT2:
			offest += (sizeof(int32_t)*2);
			break;
		case AttributeType::SINT3:
			offest += (sizeof(int32_t)*3);
			break;
		case AttributeType::SINT4:
			offest += (sizeof(int32_t)*4);
			break;
		}
	}

	MeshHandle VulkanDevice::CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount, uint32_t indexCount, uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes)
	{
		MeshHandle handle;
		VK_Mesh& mesh = m_Meshes.Emplace(handle, VK_Mesh());
		mesh.m_Buffers = std::move(buffers);
		mesh.m_VertexCount = vertexCount;
		mesh.m_IndexCount = indexCount;

		const uint32_t binding = 0;

		mesh.m_VertexDescription.binding = binding;
		mesh.m_VertexDescription.stride = stride;
		mesh.m_VertexDescription.inputRate = vk::VertexInputRate::eVertex;

		mesh.m_AttributeDescriptions.resize(attributeTypes.size());
		uint32_t currentOffset = 0;
		for (size_t i = 0; i < mesh.m_AttributeDescriptions.size(); i++)
		{
			mesh.m_AttributeDescriptions[i].binding = binding;
			mesh.m_AttributeDescriptions[i].location = i;
			mesh.m_AttributeDescriptions[i].format = GetFormat(attributeTypes[i]);
			mesh.m_AttributeDescriptions[i].offset = currentOffset;
			GetNextOffset(attributeTypes[i], currentOffset);
		}

		std::stringstream str;
		str << "VulkanDevice: Mesh " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
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
		VK_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("VulkanDevice::FreeBuffer: Invalid buffer handle.");
			return;
		}

		m_LogicalDevice.destroyBuffer(buffer->m_VKBuffer);
		m_LogicalDevice.freeMemory(buffer->m_VKMemory);

		std::stringstream str;
		str << "VulkanDevice: Buffer " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());
	}

	void VulkanDevice::FreeMesh(MeshHandle& handle)
	{
		VK_Mesh* mesh = m_Meshes.Find(handle);
		if (!mesh)
		{
			Debug().LogError("VulkanDevice::FreeMesh: Invalid mesh handle.");
			return;
		}

		for (auto& buffer : mesh->m_Buffers)
			FreeBuffer(buffer);

		m_Meshes.Erase(handle);

		std::stringstream str;
		str << "VulkanDevice: Mesh " << handle << " was freed.";
		Debug().LogInfo(str.str());

		handle = 0;
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
