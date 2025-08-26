#include "VulkanDevice.h"
#include "VulkanGraphicsModule.h"
#include "VulkanStructsConverter.h"
#include "DescriptorAllocator.h"

#include <Debug.h>
#include <Engine.h>
#include <PipelineData.h>

namespace Glory
{
	VulkanDevice::VulkanDevice(VulkanGraphicsModule* pModule, vk::PhysicalDevice physicalDevice):
		GraphicsDevice(pModule), m_VKDevice(physicalDevice), m_DidLastSupportCheckPass(false), m_DescriptorAllocator(this)
	{
		m_APIFeatures = APIFeatures::All;
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
		if (!m_Features.shaderInt64) return;
		if (!m_Features.vertexPipelineStoresAndAtomics) return;
		if (!m_Features.fragmentStoresAndAtomics) return;

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

		vk::PhysicalDeviceVulkan12Features vk12Features = vk::PhysicalDeviceVulkan12Features();
		vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();
		vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo()
			.setPQueueCreateInfos(queueCreateInfos.data())
			.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
			.setPEnabledFeatures(&deviceFeatures)
			.setEnabledExtensionCount(static_cast<uint32_t>(m_DeviceExtensions.size()))
			.setPpEnabledExtensionNames(m_DeviceExtensions.data())
			.setPNext(&vk12Features);
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.shaderInt64 = VK_TRUE;
		deviceFeatures.vertexPipelineStoresAndAtomics = VK_TRUE;
		deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
		vk12Features.separateDepthStencilLayouts = VK_TRUE;

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
		CreateCommandBuffer();
	}

	void VulkanDevice::CreateCommandBuffer()
	{
		vk::CommandPool commandPool = GetGraphicsCommandPool(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

		// Create command buffers
		m_FrameCommandBuffers.resize(1);

		vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
			.setCommandPool(commandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount((uint32_t)m_FrameCommandBuffers.size());

		if (m_LogicalDevice.allocateCommandBuffers(&commandBufferAllocateInfo, m_FrameCommandBuffers.data()) != vk::Result::eSuccess)
		{
			Debug().LogError("Vulkan: Failed to allocate command buffer.");
		}

		vk::FenceCreateInfo fenceCreateInfo = vk::FenceCreateInfo()
			.setFlags((vk::FenceCreateFlagBits)0);

		if (m_LogicalDevice.createFence(&fenceCreateInfo, nullptr, &m_Fence) != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create sync objects for a frame!");
		}
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

	void VulkanDevice::Begin()
	{
		vk::CommandBufferBeginInfo commandBeginInfo = vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
			.setPInheritanceInfo(nullptr);

		vk::CommandBuffer commandBuffer = m_FrameCommandBuffers[0];
		commandBuffer.begin(commandBeginInfo);
	}

	void VulkanDevice::BeginRenderPass(RenderPassHandle handle)
	{
		VK_RenderPass* renderPass = m_RenderPasses.Find(handle);
		if (!renderPass)
		{
			Debug().LogError("VulkanDevice::BeginRenderPass: Invalid render pass handle.");
			return;
		}
		VK_RenderTexture* renderTexture = m_RenderTextures.Find(renderPass->m_RenderTexture);
		if (!renderTexture)
		{
			Debug().LogError("VulkanDevice::BeginRenderPass: Render pass has an invalid render texture handle.");
			return;
		}

		// Start a render pass
        vk::Rect2D renderArea = vk::Rect2D()
            .setOffset(vk::Offset2D(0, 0))
            .setExtent(vk::Extent2D(renderTexture->m_Width, renderTexture->m_Height));

        std::vector<vk::ClearValue> clearColors = std::vector<vk::ClearValue>(renderTexture->m_Textures.size());
        for (size_t i = 0; i < clearColors.size(); ++i)
        {
			glm::vec4 value{0.0f, 0.0f, 0.0f, 1.0f};
            vk::ClearColorValue clearColor;
            memcpy(&clearColor, (const void*)&value, sizeof(float) * 4);
            clearColors[i].setColor(clearColor);
			//clearColors[i].setDepthStencil(vk::ClearDepthStencilValue(0.0f, 0));
        }

		if (renderTexture->m_HasDepthOrStencil)
		{
			clearColors.back().setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0));
		}

        vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
            .setRenderPass(renderPass->m_VKRenderPass)
            .setFramebuffer(renderTexture->m_VKFramebuffer)
            .setRenderArea(renderArea)
            .setClearValueCount(static_cast<uint32_t>(clearColors.size()))
            .setPClearValues(clearColors.data());

        vk::CommandBuffer commandBuffer = m_FrameCommandBuffers[0];
        commandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
	}

	void VulkanDevice::BeginPipeline(PipelineHandle handle)
	{
		VK_Pipeline* pipeline = m_Pipelines.Find(handle);
		if (!pipeline)
		{
			Debug().LogError("VulkanDevice::BeginPipeline: Invalid pipeline handle.");
			return;
		}

		vk::CommandBuffer commandBuffer = m_FrameCommandBuffers[0];
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->m_VKPipeline);
	}

	void VulkanDevice::End()
	{
		vk::CommandBuffer commandBuffer = m_FrameCommandBuffers[0];
		commandBuffer.end();

		// Submit command buffer
	    //vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
	    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	    //vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
	    vk::SubmitInfo submitInfo = vk::SubmitInfo()
	        .setWaitSemaphoreCount(0)
	        .setPWaitSemaphores(nullptr)
	        .setPWaitDstStageMask(waitStages)
	        .setCommandBufferCount(1)
	        .setPCommandBuffers(&m_FrameCommandBuffers[0])
	        .setSignalSemaphoreCount(0)
	        .setPSignalSemaphores(nullptr);

		if (m_GraphicsQueue.submit(1, &submitInfo, m_Fence) != vk::Result::eSuccess)
			throw std::runtime_error("failed to submit draw command buffer!");

		m_LogicalDevice.waitForFences(1, &m_Fence, VK_TRUE, UINT64_MAX);
		m_LogicalDevice.resetFences(1, &m_Fence);
	}

	void VulkanDevice::EndRenderPass()
	{
		vk::CommandBuffer commandBuffer = m_FrameCommandBuffers[0];
		commandBuffer.endRenderPass();
	}

	void VulkanDevice::EndPipeline()
	{
	}

	void VulkanDevice::BindDescriptorSets(PipelineHandle pipeline, std::vector<DescriptorSetHandle> sets, uint32_t firstSet)
	{
		VK_Pipeline* vkPipeline = m_Pipelines.Find(pipeline);
		if (!vkPipeline)
		{
			Debug().LogError("VulkanDevice::BindDescriptorSet: Invalid pipeline handle.");
			return;
		}

		std::vector<vk::DescriptorSet> setsToBind(sets.size());
		for (size_t i = 0; i < sets.size(); ++i)
		{
			VK_DescriptorSet* vkSet = m_DescriptorSets.Find(sets[i]);
			if (!vkSet)
			{
				Debug().LogError("VulkanDevice::BindDescriptorSet: Invalid set handle.");
				return;
			}
			setsToBind[i] = vkSet->m_VKDescriptorSet;
		}

		vk::CommandBuffer commandBuffer = m_FrameCommandBuffers[0];
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkPipeline->m_VKLayout,
			firstSet, setsToBind.size(), setsToBind.data(), 0, nullptr);
	}

	void VulkanDevice::PushConstants(PipelineHandle pipeline, uint32_t offset, uint32_t size, const void* data)
	{
		VK_Pipeline* vkPipeline = m_Pipelines.Find(pipeline);
		if (!vkPipeline)
		{
			Debug().LogError("VulkanDevice::PushConstants: Invalid pipeline handle.");
			return;
		}

		vk::CommandBuffer commandBuffer = m_FrameCommandBuffers[0];
		commandBuffer.pushConstants(vkPipeline->m_VKLayout, vk::FlagTraits<vk::ShaderStageFlagBits>::allFlags, offset, size, data);
	}

	void VulkanDevice::DrawMesh(MeshHandle handle)
	{
		VK_Mesh* mesh = m_Meshes.Find(handle);
		if (!mesh)
		{
			Debug().LogError("VulkanDevice::DrawMesh: Invalid mesh handle.");
			return;
		}

		const bool hasIndexBuffer = mesh->m_IndexCount > 0;
		const size_t vertexBufferCount = mesh->m_Buffers.size() - hasIndexBuffer ? 1 : 0;

		std::vector<vk::Buffer> vertexBuffers(vertexBufferCount);
		std::vector<vk::DeviceSize> offsets(vertexBufferCount);
		for (size_t i = 0; i < mesh->m_Buffers.size() - hasIndexBuffer ? 1 : 0; ++i)
		{
			VK_Buffer* buffer = m_Buffers.Find(mesh->m_Buffers[i]);
			vertexBuffers[i] = buffer->m_VKBuffer;
			offsets[i] = 0;
		}

		VK_Buffer* indexBuffer = mesh->m_IndexCount > 0 ? m_Buffers.Find(mesh->m_Buffers.back()) : nullptr;

		vk::CommandBuffer commandBuffer = m_FrameCommandBuffers[0];
		commandBuffer.bindVertexBuffers(0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
		if (indexBuffer)
			commandBuffer.bindIndexBuffer(indexBuffer->m_VKBuffer, 0, vk::IndexType::eUint32);

		if (hasIndexBuffer)
			commandBuffer.drawIndexed(mesh->m_IndexCount, 1, 0, 0, 0);
		else
			commandBuffer.draw(mesh->m_VertexCount, 1, 0, 0);
	}

	vk::BufferUsageFlags GetBufferUsageFlags(BufferType bufferType)
	{
		switch (bufferType)
		{
		case Glory::BT_TransferRead:
			return vk::BufferUsageFlagBits::eTransferSrc;
		case Glory::BT_TransferWrite:
			return vk::BufferUsageFlagBits::eTransferDst;
		case Glory::BT_Vertex:
			return vk::BufferUsageFlagBits::eVertexBuffer;
		case Glory::BT_Index:
			return vk::BufferUsageFlagBits::eIndexBuffer;
		case Glory::BT_Storage:
			return vk::BufferUsageFlagBits::eStorageBuffer;
		case Glory::BT_Uniform:
			return vk::BufferUsageFlagBits::eUniformBuffer;
		default:
			return vk::BufferUsageFlagBits(0);
		}
	}

	vk::DescriptorType GetDescriptorType(BufferType bufferType)
	{
		switch (bufferType)
		{
		case Glory::BT_TransferRead:
			return vk::DescriptorType::eStorageBuffer;
		case Glory::BT_TransferWrite:
			return vk::DescriptorType::eStorageBuffer;
		case Glory::BT_Vertex:
			return vk::DescriptorType::eStorageBuffer;
		case Glory::BT_Index:
			return vk::DescriptorType::eStorageBuffer;
		case Glory::BT_Storage:
			return vk::DescriptorType::eStorageBuffer;
		case Glory::BT_Uniform:
			return vk::DescriptorType::eUniformBuffer;
		default:
			return vk::DescriptorType(0);
		}
	}

	BufferHandle VulkanDevice::CreateBuffer(size_t bufferSize, BufferType type)
	{
		BufferHandle handle;
		VK_Buffer& buffer = m_Buffers.Emplace(handle, VK_Buffer());
		buffer.m_Size = bufferSize;

		vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo();
		bufferInfo.size = (vk::DeviceSize)buffer.m_Size;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;
		bufferInfo.usage = buffer.m_VKUsage = GetBufferUsageFlags(type);

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
		ImageData* pImage = pTexture->GetImageData(&m_pModule->GetEngine()->GetAssetManager());
		if (!pImage)
		{
			Debug().LogError("VulkanDevice::CreateTexture(TextureData): Could not get ImageData.");
			return NULL;
		}

		TextureCreateInfo createInfo;
		createInfo.m_Width = pImage->GetWidth();
		createInfo.m_Height = pImage->GetHeight();
		createInfo.m_ImageAspectFlags = IA_Color;
		createInfo.m_ImageType = ImageType::IT_2D;
		createInfo.m_InternalFormat = pImage->GetInternalFormat();
		createInfo.m_PixelFormat = pImage->GetFormat();
		createInfo.m_Type = pImage->GetDataType();
		createInfo.m_SamplerSettings = pTexture->GetSamplerSettings();

		return CreateTexture(createInfo, pImage->GetPixels(), pImage->DataSize());
	}

	void EnsureSupportedFormat(vk::Format& format, vk::ImageViewCreateInfo& viewInfo)
	{
		switch (format)
		{
		case vk::Format::eR8G8B8Unorm:
			format = vk::Format::eR8G8B8A8Unorm;
			viewInfo.components.a = vk::ComponentSwizzle::eOne;
			break;
		case vk::Format::eR8G8B8Srgb:
			format = vk::Format::eR8G8B8A8Srgb;
			viewInfo.components.a = vk::ComponentSwizzle::eOne;
			break;
		default:
			break;
		}
	}

	TextureHandle VulkanDevice::CreateTexture(const TextureCreateInfo& textureInfo, const void* pixels, size_t dataSize)
	{
		TextureHandle handle;
		VK_Texture& texture = m_Textures.Emplace(handle, VK_Texture());

		const vk::Format format = VKConverter::GetVulkanFormat(textureInfo.m_InternalFormat); //vk::Format::eR8G8B8A8Srgb;
		const vk::ImageType imageType = VKConverter::GetVulkanImageType(textureInfo.m_ImageType);
		vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo();
		imageInfo.imageType = imageType;
		imageInfo.extent.width = textureInfo.m_Width;
		imageInfo.extent.height = textureInfo.m_Height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = VKConverter::GetVulkanImageUsageFlags(textureInfo.m_ImageAspectFlags);
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.flags = (vk::ImageCreateFlags)0;

		vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo();
		EnsureSupportedFormat(imageInfo.format, viewInfo);

		const vk::ImageAspectFlags imageAspect = VKConverter::GetVulkanImageAspectFlags(textureInfo.m_ImageAspectFlags);

		if (m_LogicalDevice.createImage(&imageInfo, nullptr, &texture.m_VKImage) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateTexture: Could not create image.");
			m_Textures.Erase(handle);
			return NULL;
		}

		vk::MemoryRequirements memRequirements;
		m_LogicalDevice.getImageMemoryRequirements(texture.m_VKImage, &memRequirements);

		const uint32_t typeFilter = memRequirements.memoryTypeBits;
		const vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		const uint32_t memoryIndex = GetSupportedMemoryIndex(typeFilter, properties);

		vk::MemoryAllocateInfo imageAllocInfo = vk::MemoryAllocateInfo();
		imageAllocInfo.allocationSize = memRequirements.size;
		imageAllocInfo.memoryTypeIndex = memoryIndex;

		if (m_LogicalDevice.allocateMemory(&imageAllocInfo, nullptr, &texture.m_VKMemory) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateTexture: Could not create image.");
			m_LogicalDevice.destroyImage(texture.m_VKImage, nullptr);
			m_Textures.Erase(handle);
			return NULL;
		}
		m_LogicalDevice.bindImageMemory(texture.m_VKImage, texture.m_VKMemory, 0);

		/* Copy buffer to image */
		if (pixels)
		{
			/* Transition image layout */
			TransitionImageLayout(texture.m_VKImage, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, imageAspect);

			const vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			BufferHandle stagingBuffer = CreateBuffer(memRequirements.size, BufferType::BT_TransferRead);//new VulkanBuffer(imageSize, (uint32_t)vk::BufferUsageFlagBits::eTransferSrc, (uint32_t)memoryFlags);
			VK_Buffer* vkStagingBuffer = m_Buffers.Find(stagingBuffer);
			AssignBuffer(stagingBuffer, pixels, uint32_t(dataSize));
			CopyFromBuffer(vkStagingBuffer->m_VKBuffer, texture.m_VKImage, imageAspect, textureInfo.m_Width, textureInfo.m_Height);

			/* Transtion layout again so it can be sampled */
			TransitionImageLayout(texture.m_VKImage, format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, imageAspect);
			
			FreeBuffer(stagingBuffer);
		}

		/* Create texture image view */
		viewInfo.image = texture.m_VKImage;
		viewInfo.viewType = VKConverter::GetVulkanImageViewType(textureInfo.m_ImageType);
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = imageAspect;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (m_LogicalDevice.createImageView(&viewInfo, nullptr, &texture.m_VKImageView) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateTexture: Could not create image view.");
			return NULL;
		}

		auto samplerIter = m_CachedSamlers.find(textureInfo.m_SamplerSettings);
		if (samplerIter == m_CachedSamlers.end())
		{
			auto samplerCreateInfo = VKConverter::GetVulkanSamplerInfo(textureInfo.m_SamplerSettings);

			vk::Sampler newSampler;
			if (m_LogicalDevice.createSampler(&samplerCreateInfo, nullptr, &newSampler) != vk::Result::eSuccess)
			{
				Debug().LogError("VulkanDevice::CreateTexture: Could not create image sampler.");
				return NULL;
			}
			samplerIter = m_CachedSamlers.emplace(textureInfo.m_SamplerSettings, newSampler).first;
		}
		texture.m_VKSampler = samplerIter->second;

		std::stringstream str;
		str << "VulkanDevice: Texture " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	RenderTextureHandle VulkanDevice::CreateRenderTexture(RenderPassHandle renderPass, const RenderTextureCreateInfo& info)
	{
		if (info.Width == 0 || info.Height == 0)
		{
			Debug().LogError("VulkanDevice::CreateRenderTexture: Invalid RenderTexture size.");
			return NULL;
		}

		VK_RenderPass* vkRenderPass = m_RenderPasses.Find(renderPass);
		if (!vkRenderPass)
		{
			Debug().LogError("VulkanDevice::CreateRenderTexture: Invalid render pass handle");
			return NULL;
		}

		RenderTextureHandle handle;
		VK_RenderTexture& renderTexture = m_RenderTextures.Emplace(handle, VK_RenderTexture());
		renderTexture.m_RenderPass = renderPass;
		renderTexture.m_Width = info.Width;
		renderTexture.m_Height = info.Height;
		renderTexture.m_HasDepthOrStencil = info.HasDepth || info.HasStencil;

		const size_t numAttachments = info.Attachments.size() + (renderTexture.m_HasDepthOrStencil ? 1 : 0);
		renderTexture.m_AttachmentNames.resize(numAttachments);
		renderTexture.m_Textures.resize(numAttachments);

		SamplerSettings sampler;
		sampler.MipmapMode = Filter::F_None;
		sampler.MinFilter = Filter::F_Nearest;
		sampler.MagFilter = Filter::F_Nearest;

		size_t textureCounter = 0;
		for (size_t i = 0; i < info.Attachments.size(); ++i)
		{
			Attachment attachment = info.Attachments[i];
			renderTexture.m_Textures[i] = CreateTexture({ info.Width, info.Height, attachment.Format, attachment.InternalFormat, attachment.ImageType, attachment.m_Type, 0, 0, attachment.ImageAspect, sampler });
			renderTexture.m_AttachmentNames[i] = attachment.Name;
			++textureCounter;
		}

		size_t depthStencilIndex = 0;

		if (info.HasDepth && info.HasStencil)
		{
			depthStencilIndex = textureCounter;
			renderTexture.m_Textures[depthStencilIndex] = CreateTexture({ info.Width, info.Height, PixelFormat::PF_Depth, PixelFormat::PF_D32SfloatS8Uint, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Depth, sampler });
			renderTexture.m_AttachmentNames[depthStencilIndex] = "DepthStencil";
			++textureCounter;
		}
		else if (info.HasDepth)
		{
			depthStencilIndex = textureCounter;
			renderTexture.m_Textures[depthStencilIndex] = CreateTexture({ info.Width, info.Height, PixelFormat::PF_Depth, PixelFormat::PF_D32Sfloat, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Depth, sampler });
			renderTexture.m_AttachmentNames[depthStencilIndex] = "Depth";
			++textureCounter;
		}
		else if (info.HasStencil)
		{
			depthStencilIndex = textureCounter;
			renderTexture.m_Textures[depthStencilIndex] = CreateTexture({ info.Width, info.Height, PixelFormat::PF_Stencil, PixelFormat::PF_R8Uint, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Stencil, sampler });
			renderTexture.m_AttachmentNames[depthStencilIndex] = "Stencil";
			++textureCounter;
		}

		std::vector<vk::ImageView> attachments(renderTexture.m_Textures.size());
		for (size_t i = 0; i < renderTexture.m_Textures.size(); ++i)
		{
			TextureHandle textureHandle = renderTexture.m_Textures[i];
			VK_Texture* texture = m_Textures.Find(textureHandle);
			attachments[i] = texture->m_VKImageView;
		}

		if (info.HasDepth || info.HasStencil)
		{
			VK_Texture* texture = m_Textures.Find(renderTexture.m_Textures[depthStencilIndex]);
			attachments[depthStencilIndex] = texture->m_VKImageView;
		}

		vk::FramebufferCreateInfo frameBufferCreateInfo = vk::FramebufferCreateInfo()
			.setRenderPass(vkRenderPass->m_VKRenderPass)
			.setAttachmentCount(attachments.size())
			.setPAttachments(attachments.data())
			.setWidth(info.Width)
			.setHeight(info.Height)
			.setLayers(1);

		renderTexture.m_VKFramebuffer = m_LogicalDevice.createFramebuffer(frameBufferCreateInfo);
		if (renderTexture.m_VKFramebuffer == nullptr)
		{
			Debug().LogError("VulkanDevice::CreateRenderTexture: There was an error when trying to create a frame buffer.");
			return NULL;
		}

		std::stringstream str;
		str << "VulkanDevice: RenderTexture " << handle << " created with " << renderTexture.m_Textures.size() << " attachments.";
		Debug().LogInfo(str.str());

		return handle;
	}

	RenderPassHandle VulkanDevice::CreateRenderPass(const RenderPassInfo& info)
	{
		if (info.RenderTextureInfo.Width == 0 || info.RenderTextureInfo.Height == 0)
		{
			Debug().LogError("VulkanDevice::CreateRenderPass: Invalid RenderTexture size.");
			return NULL;
		}

		RenderPassHandle handle;
		VK_RenderPass& renderPass = m_RenderPasses.Emplace(handle, VK_RenderPass());

		std::vector<vk::AttachmentDescription> attachments;
		std::vector<vk::AttachmentReference> attachmentColorRefs;
		vk::AttachmentReference attachmentDepthStencilRef;
		const size_t attachmentCount = info.RenderTextureInfo.Attachments.size();
		attachments.resize(attachmentCount + (
			info.RenderTextureInfo.HasDepth || info.RenderTextureInfo.HasStencil ? 1 : 0));
		attachmentColorRefs.resize(attachmentCount);
		for (size_t i = 0; i < attachmentCount; ++i)
		{
			const Attachment& attachment = info.RenderTextureInfo.Attachments[i];

			const vk::Format format = VKConverter::GetVulkanFormat(attachment.InternalFormat);

			// Create render pass
			attachments[i] = vk::AttachmentDescription()
				.setFormat(format)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(vk::ImageLayout::eUndefined)
				.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

			attachmentColorRefs[i] = vk::AttachmentReference()
				.setAttachment(i)
				.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
		}

		if (info.RenderTextureInfo.HasDepth && info.RenderTextureInfo.HasStencil)
		{
			attachments[attachmentCount] = vk::AttachmentDescription()
				.setFormat(vk::Format::eD32SfloatS8Uint)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(vk::ImageLayout::eUndefined)
				.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			attachmentDepthStencilRef = vk::AttachmentReference()
				.setAttachment(attachmentCount)
				.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		}
		else if (info.RenderTextureInfo.HasDepth)
		{
			attachments[attachmentCount] = vk::AttachmentDescription()
				.setFormat(vk::Format::eD32Sfloat)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(vk::ImageLayout::eUndefined)
				.setFinalLayout(vk::ImageLayout::eDepthAttachmentOptimal);

			attachmentDepthStencilRef = vk::AttachmentReference()
				.setAttachment(attachmentCount)
				.setLayout(vk::ImageLayout::eDepthAttachmentOptimal);
		}
		else if (info.RenderTextureInfo.HasStencil)
		{
			attachments[attachmentCount] = vk::AttachmentDescription()
				.setFormat(vk::Format::eS8Uint)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(vk::ImageLayout::eUndefined)
				.setFinalLayout(vk::ImageLayout::eStencilAttachmentOptimal);

			attachmentDepthStencilRef = vk::AttachmentReference()
				.setAttachment(attachmentCount)
				.setLayout(vk::ImageLayout::eStencilAttachmentOptimal);
		}

		vk::SubpassDescription subPass = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(attachmentColorRefs.size())
			.setPColorAttachments(attachmentColorRefs.data());
		if (info.RenderTextureInfo.HasDepth || info.RenderTextureInfo.HasStencil)
			subPass.setPDepthStencilAttachment(&attachmentDepthStencilRef);

		vk::SubpassDependency dependancy = vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setSrcAccessMask((vk::AccessFlags)0)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

		vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo()
			.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
			.setPAttachments(attachments.data())
			.setSubpassCount(1)
			.setPSubpasses(&subPass)
			.setDependencyCount(1)
			.setPDependencies(&dependancy);

		renderPass.m_VKRenderPass = m_LogicalDevice.createRenderPass(renderPassCreateInfo);
		if (renderPass.m_VKRenderPass == nullptr)
		{
			Debug().LogError("VulkanDevice::CreateRenderPass: Failed to create render pass.");
			m_RenderPasses.Erase(handle);
			return NULL;
		}

		renderPass.m_RenderTexture = CreateRenderTexture(handle, info.RenderTextureInfo);

		if (renderPass.m_RenderTexture == NULL)
		{
			m_RenderPasses.Erase(handle);
			Debug().LogError("VulkanDevice::CreateRenderPass: Failed to create RenderTexture for RenderPass.");
			return NULL;
		}

		std::stringstream str;
		str << "VulkanDevice: RenderPass " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	ShaderHandle VulkanDevice::CreateShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function)
	{
		ShaderHandle handle;
		VK_Shader& shader = m_Shaders.Emplace(handle, VK_Shader());

		vk::ShaderModuleCreateInfo shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(pShaderFileData->Size())
			.setPCode(reinterpret_cast<const uint32_t*>(pShaderFileData->Data()));
		shader.m_Function = function;
		shader.m_VKStage = VKConverter::GetShaderStageFlag(shaderType);
		shader.m_VKModule = m_LogicalDevice.createShaderModule(shaderModuleCreateInfo, nullptr);

		std::stringstream str;
		str << "VulkanDevice: Shader " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	PipelineHandle VulkanDevice::CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline,
		std::vector<DescriptorSetLayoutHandle>&& descriptorSetLayouts, size_t stride, const std::vector<AttributeType>& attributeTypes)
	{
		PipelineManager& pipelines = m_pModule->GetEngine()->GetPipelineManager();
		std::vector<vk::PushConstantRange> pushConstants;

		VK_RenderPass* vkRenderPass = m_RenderPasses.Find(renderPass);
		if (!vkRenderPass)
		{
			Debug().LogError("VulkanDevice::CreatePipeline: Invalid render pass handle.");
			return NULL;
		}

		VK_RenderTexture* vkRenderTexture = m_RenderTextures.Find(vkRenderPass->m_RenderTexture);
		if (!vkRenderTexture)
		{
			Debug().LogError("VulkanDevice::CreatePipeline: Invalid render texture handle in render pass.");
			return NULL;
		}

		std::vector<vk::DescriptorSetLayout> vkDescriptorSetLayouts(descriptorSetLayouts.size());
		for (size_t i = 0; i < descriptorSetLayouts.size(); ++i)
		{
			VK_DescriptorSetLayout* vkSetLayout = m_DescriptorSetLayouts.Find(descriptorSetLayouts[i]);
			if (!vkSetLayout)
			{
				Debug().LogError("VulkanDevice::CreatePipeline: Invalid descriptor set layout handle.");
				return NULL;
			}
			vkDescriptorSetLayouts[i] = vkSetLayout->m_VKLayout;

			if (vkSetLayout->m_PushConstantRange.size)
				pushConstants.push_back(vkSetLayout->m_PushConstantRange);
		}

		PipelineHandle handle;
		VK_Pipeline& pipeline = m_Pipelines.Emplace(handle, VK_Pipeline());
		pipeline.m_RenderPass = renderPass;

		const uint32_t binding = 0;

		pipeline.m_VertexDescription.binding = binding;
		pipeline.m_VertexDescription.stride = stride;
		pipeline.m_VertexDescription.inputRate = vk::VertexInputRate::eVertex;

		pipeline.m_AttributeDescriptions.resize(attributeTypes.size());
		uint32_t currentOffset = 0;
		for (size_t i = 0; i < pipeline.m_AttributeDescriptions.size(); i++)
		{
			pipeline.m_AttributeDescriptions[i].binding = binding;
			pipeline.m_AttributeDescriptions[i].location = i;
			pipeline.m_AttributeDescriptions[i].format = GetFormat(attributeTypes[i]);
			pipeline.m_AttributeDescriptions[i].offset = currentOffset;
			GetNextOffset(attributeTypes[i], currentOffset);
		}

		pipeline.m_Shaders.resize(pPipeline->ShaderCount());
		for (size_t i = 0; i < pipeline.m_Shaders.size(); ++i)
			pipeline.m_Shaders[i] = CreateShader(pPipeline->Shader(pipelines, i), pPipeline->GetShaderType(pipelines, i), "main");

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(pPipeline->ShaderCount());
		for (size_t i = 0; i < shaderStages.size(); ++i)
		{
			VK_Shader* shader = m_Shaders.Find(pipeline.m_Shaders[i]);
			shaderStages[i] = vk::PipelineShaderStageCreateInfo()
				.setStage(shader->m_VKStage)
				.setModule(shader->m_VKModule)
				.setPName(shader->m_Function.data());
		}

		/*const size_t numLayouts = pPipeline->UniformBufferCount() + pPipeline->StorageBufferCount();
		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(numLayouts);
		size_t layoutIndex = 0;
		for (size_t i = 0; i < pPipeline->UniformBufferCount(); ++i)
		{
			const ShaderBufferInfo& bufferInfo = pPipeline->UniformBuffer(i);
			layoutBindings[layoutIndex].binding = BindingIndex(bufferInfo.Name);
			layoutBindings[layoutIndex].descriptorType = vk::DescriptorType::eUniformBuffer;
			layoutBindings[layoutIndex].descriptorCount = 1;
			layoutBindings[layoutIndex].stageFlags = VKConverter::ToShaderStageFlags(bufferInfo.ShaderFlags);
			++layoutIndex;
		}

		for (size_t i = 0; i < pPipeline->StorageBufferCount(); ++i)
		{
			const ShaderBufferInfo& bufferInfo = pPipeline->StorageBuffer(i);
			layoutBindings[layoutIndex].binding = BindingIndex(bufferInfo.Name);
			layoutBindings[layoutIndex].descriptorType = vk::DescriptorType::eStorageBuffer;
			layoutBindings[layoutIndex].descriptorCount = 1;
			layoutBindings[layoutIndex].stageFlags = VKConverter::ToShaderStageFlags(bufferInfo.ShaderFlags);
			++layoutIndex;
		}

		vk::DescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.bindingCount = layoutBindings.size();
		layoutInfo.pBindings = layoutBindings.data();

		if (m_LogicalDevice.createDescriptorSetLayout(&layoutInfo, nullptr, &pipeline.m_VKDescriptorSetLayouts) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to create descriptor set layout!");*/

		//m_DescriptorAllocator.Allocate(&pipeline.m_VKBuffersDescriptorSet, pipeline.m_VKDescriptorSetLayouts);

		// Vertex input state
		vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptionCount(1)
			.setPVertexBindingDescriptions(&pipeline.m_VertexDescription)
			.setVertexAttributeDescriptionCount(static_cast<uint32_t>(pipeline.m_AttributeDescriptions.size()))
			.setPVertexAttributeDescriptions(pipeline.m_AttributeDescriptions.data());

		// Input assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(VK_FALSE);

		// Viewport and scissor
		vk::Viewport viewport = vk::Viewport()
			.setX(0.0f)
			.setY(0.0f)
			.setWidth((float)vkRenderTexture->m_Width)
			.setHeight((float)vkRenderTexture->m_Height)
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f);

		vk::Rect2D scissor = vk::Rect2D()
			.setOffset({ 0,0 })
			.setExtent({ vkRenderTexture->m_Width, vkRenderTexture->m_Height });

		vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo()
			.setViewportCount(1)
			.setPViewports(&viewport)
			.setScissorCount(1)
			.setPScissors(&scissor);

		// Rasterizer state
		vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo()
			.setDepthClampEnable(VK_FALSE) // Requires a GPU feature
			.setRasterizerDiscardEnable(VK_FALSE)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.0f)
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eClockwise)
			.setDepthBiasEnable(VK_FALSE)
			.setDepthBiasConstantFactor(0.0f)
			.setDepthBiasClamp(0.0f)
			.setDepthBiasSlopeFactor(0.0f);

		// Multisampling state
		vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo()
			.setSampleShadingEnable(VK_FALSE)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setMinSampleShading(1.0f)
			.setPSampleMask(nullptr)
			.setAlphaToCoverageEnable(VK_FALSE)
			.setAlphaToOneEnable(VK_FALSE);

		// Blend state
		const size_t attachmentCount = vkRenderTexture->m_Textures.size() - (vkRenderTexture->m_HasDepthOrStencil ? 1 : 0);
		std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates(attachmentCount);
		for (size_t i = 0; i < attachmentCount; ++i)
		{
			colorBlendAttachmentStates[i] = vk::PipelineColorBlendAttachmentState()
				.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
				.setBlendEnable(VK_FALSE)
				.setSrcColorBlendFactor(vk::BlendFactor::eOne)
				.setDstColorBlendFactor(vk::BlendFactor::eZero)
				.setColorBlendOp(vk::BlendOp::eAdd)
				.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
				.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
				.setAlphaBlendOp(vk::BlendOp::eAdd);
		}

		vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(VK_FALSE)
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachmentCount(colorBlendAttachmentStates.size())
			.setPAttachments(colorBlendAttachmentStates.data())
			.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

		vk::PipelineDepthStencilStateCreateInfo depthStencil = vk::PipelineDepthStencilStateCreateInfo();
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = vk::CompareOp::eLess;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		//depthStencil.front = {}; // Optional
		//depthStencil.back = {}; // Optional

		// Dynamic state
		//vk::DynamicState dynamicStates[] = {
		//    vk::DynamicState::eViewport,
		//    vk::DynamicState::eLineWidth
		//};
		//
		//vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
		//    .setDynamicStateCount(2)
		//    .setPDynamicStates(dynamicStates);

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(static_cast<uint32_t>(vkDescriptorSetLayouts.size()))
			.setPSetLayouts(vkDescriptorSetLayouts.data())
			.setPushConstantRangeCount(static_cast<uint32_t>(pushConstants.size()))
			.setPPushConstantRanges(pushConstants.data());

		pipeline.m_VKLayout = m_LogicalDevice.createPipelineLayout(pipelineLayoutCreateInfo);
		if (pipeline.m_VKLayout == nullptr)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

		// Create the pipeline
		vk::GraphicsPipelineCreateInfo pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			.setStageCount(static_cast<uint32_t>(shaderStages.size()))
			.setPStages(shaderStages.data())
			.setPVertexInputState(&vertexInputStateCreateInfo)
			.setPInputAssemblyState(&inputAssemblyStateCreateInfo)
			.setPViewportState(&viewportStateCreateInfo)
			.setPRasterizationState(&rasterizationStateCreateInfo)
			.setPMultisampleState(&multisampleStateCreateInfo)
			.setPDepthStencilState(&depthStencil)
			.setPColorBlendState(&colorBlendStateCreateInfo)
			.setPDynamicState(nullptr)
			.setLayout(pipeline.m_VKLayout)
			.setRenderPass(vkRenderPass->m_VKRenderPass)
			.setSubpass(0)
			.setBasePipelineHandle(VK_NULL_HANDLE)
			.setBasePipelineIndex(-1);

		if (m_LogicalDevice.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline.m_VKPipeline) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreatePipeline: Failed to create graphics pipeline.");
			return NULL;
		}

		std::stringstream str;
		str << "VulkanDevice: Pipeline " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	DescriptorSetLayoutHandle VulkanDevice::CreateDescriptorSetLayout(DescriptorSetLayoutInfo&& setLayoutInfo)
	{
		auto iter = m_CachedDescriptorSetLayouts.find(setLayoutInfo);
		if (iter == m_CachedDescriptorSetLayouts.end())
		{
			std::vector<uint32_t> bindingIndices;
			std::vector<vk::DescriptorType> descriptorTypes;
			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(setLayoutInfo.m_Buffers.size() +
				setLayoutInfo.m_Samplers.size());
			for (size_t i = 0; i < setLayoutInfo.m_Buffers.size(); ++i)
			{
				auto& bufferInfo = setLayoutInfo.m_Buffers[i];

				layoutBindings[i].descriptorType = GetDescriptorType(bufferInfo.m_Type);
				layoutBindings[i].binding = bufferInfo.m_BindingIndex;
				layoutBindings[i].descriptorCount = 1;
				/** @todo: Pass for which shaders stages the buffer is meant for */
				layoutBindings[i].stageFlags = vk::FlagTraits<vk::ShaderStageFlagBits>::allFlags;
				bindingIndices.emplace_back(bufferInfo.m_BindingIndex);
				descriptorTypes.emplace_back(layoutBindings[i].descriptorType);
			}

			for (size_t i = 0; i < setLayoutInfo.m_Samplers.size(); ++i)
			{
				const size_t index = setLayoutInfo.m_Buffers.size() + i;
				layoutBindings[index].binding = setLayoutInfo.m_Samplers[i].m_BindingIndex;
				layoutBindings[index].descriptorCount = 1;
				layoutBindings[index].descriptorType = vk::DescriptorType::eCombinedImageSampler;
				layoutBindings[index].pImmutableSamplers = nullptr;
				layoutBindings[index].stageFlags = vk::ShaderStageFlagBits::eAll;
				bindingIndices.emplace_back(layoutBindings[index].binding);
				descriptorTypes.emplace_back(layoutBindings[index].descriptorType);
			}

			vk::DescriptorSetLayout layout = nullptr;
			if (!layoutBindings.empty())
			{
				vk::DescriptorSetLayoutCreateInfo layoutInfo{};
				layoutInfo.bindingCount = layoutBindings.size();
				layoutInfo.pBindings = layoutBindings.data();
				if (m_LogicalDevice.createDescriptorSetLayout(&layoutInfo, nullptr, &layout) != vk::Result::eSuccess)
				{
					Debug().LogError("VulkanDevice::CreateDescriptorSet: Failed to create descriptor set layout.");
					return NULL;
				}
			}

			iter = m_CachedDescriptorSetLayouts.emplace(setLayoutInfo, UUID()).first;
			VK_DescriptorSetLayout& setLayout = m_DescriptorSetLayouts.Emplace(iter->second, VK_DescriptorSetLayout());
			setLayout.m_VKLayout = layout;
			setLayout.m_PushConstantRange.offset = setLayoutInfo.m_PushConstantRange.m_Offset;
			setLayout.m_PushConstantRange.size = setLayoutInfo.m_PushConstantRange.m_Size;
			setLayout.m_PushConstantRange.stageFlags = vk::FlagTraits<vk::ShaderStageFlagBits>::allFlags;
			setLayout.m_BindingIndices = std::move(bindingIndices);
			setLayout.m_DescriptorTypes = std::move(descriptorTypes);
		}
		return iter->second;
	}

	DescriptorSetHandle VulkanDevice::CreateDescriptorSet(DescriptorSetInfo&& setInfo)
	{
		VK_DescriptorSetLayout* vkDescriptorSetLayout = m_DescriptorSetLayouts.Find(setInfo.m_Layout);
		if (!vkDescriptorSetLayout)
		{
			Debug().LogError("VulkanDevice::CreateDescriptorSet: Invalid descriptor set layout handle.");
			return NULL;
		}

		std::vector<vk::WriteDescriptorSet> descriptorWrites(setInfo.m_Buffers.size() + setInfo.m_Samplers.size());
		std::vector<vk::DescriptorBufferInfo> bufferInfos(setInfo.m_Buffers.size());
		std::vector<vk::DescriptorImageInfo> imageInfos(setInfo.m_Samplers.size());
		uint32_t descriptorIndex = 0;
		for (size_t i = 0; i < setInfo.m_Buffers.size(); ++i)
		{
			auto& bufferInfo = setInfo.m_Buffers[i];
			VK_Buffer* vkBuffer = m_Buffers.Find(setInfo.m_Buffers[i].m_BufferHandle);
			if (!vkBuffer)
			{
				Debug().LogError("VulkanDevice::CreateDescriptorSet: Invalid buffer handle.");
				return NULL;
			}

			bufferInfos[i].buffer = vkBuffer->m_VKBuffer;
			bufferInfos[i].offset = bufferInfo.m_Offset;
			bufferInfos[i].range = bufferInfo.m_Size;

			descriptorWrites[i].dstBinding = vkDescriptorSetLayout->m_BindingIndices[descriptorIndex];
			descriptorWrites[i].dstArrayElement = 0;
			descriptorWrites[i].descriptorType = vkDescriptorSetLayout->m_DescriptorTypes[descriptorIndex];
			descriptorWrites[i].descriptorCount = 1;
			descriptorWrites[i].pBufferInfo = &bufferInfos[i];
			descriptorWrites[i].pImageInfo = nullptr;
			descriptorWrites[i].pTexelBufferView = nullptr;
			++descriptorIndex;
		}

		for (size_t i = 0; i < setInfo.m_Samplers.size(); ++i)
		{
			const size_t index = setInfo.m_Buffers.size() + i;
			auto& samplerInfo = setInfo.m_Samplers[i];
			VK_Texture* vkTexture = m_Textures.Find(setInfo.m_Samplers[i].m_TextureHandle);

			imageInfos[i].imageLayout = vkTexture ? vkTexture->m_VKLayout : vk::ImageLayout::eUndefined;
			imageInfos[i].imageView = vkTexture ? vkTexture->m_VKImageView : nullptr;
			imageInfos[i].sampler = vkTexture ? vkTexture->m_VKSampler : nullptr;

			descriptorWrites[index].dstBinding = vkDescriptorSetLayout->m_BindingIndices[descriptorIndex];
			descriptorWrites[index].dstArrayElement = 0;
			descriptorWrites[index].descriptorType = vkDescriptorSetLayout->m_DescriptorTypes[descriptorIndex];
			descriptorWrites[index].descriptorCount = 1;
			descriptorWrites[index].pImageInfo = &imageInfos[i];
			++descriptorIndex;
		}

		DescriptorSetHandle handle;
		VK_DescriptorSet& set = m_DescriptorSets.Emplace(handle, VK_DescriptorSet());
		set.m_Layout = setInfo.m_Layout;

		m_DescriptorAllocator.Allocate(&set.m_VKDescriptorSet, vkDescriptorSetLayout->m_VKLayout);
		for (size_t i = 0; i < descriptorWrites.size(); ++i)
			descriptorWrites[i].dstSet = set.m_VKDescriptorSet;
		m_LogicalDevice.updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

		return handle;
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
		VK_Texture* texture = m_Textures.Find(handle);
		if (!texture)
		{
			Debug().LogError("VulkanDevice::FreeTexture: Invalid texture handle.");
			return;
		}

		m_LogicalDevice.destroyImageView(texture->m_VKImageView, nullptr);
		m_LogicalDevice.destroyImage(texture->m_VKImage, nullptr);
		m_LogicalDevice.freeMemory(texture->m_VKMemory, nullptr);

		m_Textures.Erase(handle);

		std::stringstream str;
		str << "VulkanDevice: Texture " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}

	void VulkanDevice::FreeRenderTexture(RenderTextureHandle& handle)
	{
		VK_RenderTexture* renderTexture = m_RenderTextures.Find(handle);
		if (!renderTexture)
		{
			Debug().LogError("VulkanDevice::FreeRenderTexture: Invalid render texture handle.");
			return;
		}

		m_LogicalDevice.destroyFramebuffer(renderTexture->m_VKFramebuffer);

		for (auto texture : renderTexture->m_Textures)
		{
			FreeTexture(texture);
		}

		renderTexture->m_Textures.clear();
		renderTexture->m_AttachmentNames.clear();

		m_RenderTextures.Erase(handle);

		std::stringstream str;
		str << "VulkanDevice: RenderTexture " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}

	void VulkanDevice::FreeRenderPass(RenderPassHandle& handle)
	{
		VK_RenderPass* renderPass = m_RenderPasses.Find(handle);
		if (!renderPass)
		{
			Debug().LogError("VulkanDevice::FreeRenderPass: Invalid render pass handle.");
			return;
		}

		FreeRenderTexture(renderPass->m_RenderTexture);
		m_LogicalDevice.destroyRenderPass(renderPass->m_VKRenderPass);

		m_RenderPasses.Erase(handle);

		std::stringstream str;
		str << "VulkanDevice: RenderPass " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}

	void VulkanDevice::FreeShader(ShaderHandle& handle)
	{
		VK_Shader* shader = m_Shaders.Find(handle);
		if (!shader)
		{
			Debug().LogError("VulkanDevice::FreeShader: Invalid shader handle.");
			return;
		}

		m_LogicalDevice.destroyShaderModule(shader->m_VKModule);

		m_Shaders.Erase(handle);

		std::stringstream str;
		str << "OpenGLDevice: Shader " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}

	void VulkanDevice::FreePipeline(PipelineHandle& handle)
	{
		VK_Pipeline* pipeline = m_Pipelines.Find(handle);
		if (!pipeline)
		{
			Debug().LogError("VulkanDevice::FreePipeline: Invalid pipeline handle.");
			return;
		}

		m_LogicalDevice.destroyDescriptorSetLayout(pipeline->m_VKDescriptorSetLayouts);
		m_LogicalDevice.destroyPipeline(pipeline->m_VKPipeline);
		m_LogicalDevice.destroyPipelineLayout(pipeline->m_VKLayout);
		m_Pipelines.Erase(handle);

		std::stringstream str;
		str << "VulkanDevice: Pipeline " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}

	vk::CommandBuffer VulkanDevice::BeginSingleTimeCommands()
	{
		vk::CommandPool commandPool = GetGraphicsCommandPool();

		vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo();
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		vk::CommandBuffer commandBuffer;
		if (m_LogicalDevice.allocateCommandBuffers(&allocInfo, &commandBuffer) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to allocate command buffer!");

		vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		commandBuffer.begin(beginInfo);
		return commandBuffer;
	}

	void VulkanDevice::EndSingleTimeCommands(vk::CommandBuffer commandBuffer)
	{
		vk::CommandPool commandPool = GetGraphicsCommandPool();

		commandBuffer.end();

		vk::SubmitInfo submitInfo = vk::SubmitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		GraphicsQueue().submit(1, &submitInfo, VK_NULL_HANDLE);
		GraphicsQueue().waitIdle();
		m_LogicalDevice.freeCommandBuffers(commandPool, 1, &commandBuffer);
	}

	void VulkanDevice::TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags)
	{
		vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

		vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier();
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspectFlags;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = (vk::AccessFlags)0;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			barrier.srcAccessMask = (vk::AccessFlags)0;
			barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else
		{
			throw std::invalid_argument("Unsupported layout transition!");
		}

		commandBuffer.pipelineBarrier(
			sourceStage, destinationStage,
			(vk::DependencyFlags)0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndSingleTimeCommands(commandBuffer);
	}

	void VulkanDevice::CopyFromBuffer(vk::Buffer buffer, vk::Image image, vk::ImageAspectFlags aspectFlags, uint32_t width, uint32_t height)
	{
		CopyFromBuffer(buffer, image, aspectFlags, 0, 0, 0, width, height, 1);
	}

	void VulkanDevice::CopyFromBuffer(vk::Buffer buffer, vk::Image image, vk::ImageAspectFlags aspectFlags, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth)
	{
		vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

		vk::BufferImageCopy copyRegion = vk::BufferImageCopy();
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = aspectFlags;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;

		copyRegion.imageOffset = vk::Offset3D(offsetX, offsetY, offsetZ);
		copyRegion.imageExtent = vk::Extent3D(width, height, depth);

		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}
}
