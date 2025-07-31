#include "VulkanDevice.h"
#include "VulkanGraphicsModule.h"
#include "VulkanStructsConverter.h"

#include <Debug.h>
#include <Engine.h>
#include <PipelineData.h>

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
		TextureHandle handle;
		VK_Texture& texture = m_Textures.Emplace(handle, VK_Texture());

		vk::Format format = VKConverter::GetVulkanFormat(textureInfo.m_PixelFormat); //vk::Format::eR8G8B8A8Srgb;
		vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo();
		vk::ImageType imageType = VKConverter::GetVulkanImageType(textureInfo.m_ImageType);
		imageInfo.imageType = imageType;
		imageInfo.extent.width = textureInfo.m_Width;
		imageInfo.extent.height = textureInfo.m_Height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = format == vk::Format::eR8G8B8A8Srgb ? vk::ImageTiling::eOptimal : (vk::ImageTiling)0;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = VKConverter::GetVulkanImageUsageFlags(textureInfo.m_ImageAspectFlags);
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.flags = (vk::ImageCreateFlags)0;

		if (m_LogicalDevice.createImage(&imageInfo, nullptr, &texture.m_VKImage) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateTexture: Could not create image.");
			m_Textures.Erase(handle);
			return NULL;
		}

		vk::MemoryRequirements memRequirements;
		m_LogicalDevice.getImageMemoryRequirements(texture.m_VKImage, &memRequirements);

		uint32_t typeFilter = memRequirements.memoryTypeBits;
		vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		uint32_t memoryIndex = GetSupportedMemoryIndex(typeFilter, properties);

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

		// Copy buffer to image
		if (pixels)
		{
			// Transition image layout
			//pGraphics->TransitionImageLayout(texture.m_VKImage, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
			//
			//vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			//BufferHandle stagingBuffer = CreateBuffer(uint32_t(memRequirements.size), BufferType::BT_TransferRead);//new VulkanBuffer(imageSize, (uint32_t)vk::BufferUsageFlagBits::eTransferSrc, (uint32_t)memoryFlags);
			//AssignBuffer(stagingBuffer, pixels);
			//Texture::CopyFromBuffer(pTextureStagingBuffer);
			//
			//// Transtion layout again so it can be sampled
			//pGraphics->TransitionImageLayout(texture.m_VKImage, format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
			//
			//FreeBuffer(stagingBuffer);
		}

		// Create texture image view
		vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo();
		viewInfo.image = texture.m_VKImage;
		viewInfo.viewType = VKConverter::GetVulkanImageViewType(textureInfo.m_ImageType);
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VKConverter::GetVulkanImageAspectFlags(textureInfo.m_ImageAspectFlags);
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (m_LogicalDevice.createImageView(&viewInfo, nullptr, &texture.m_VKImageView) != vk::Result::eSuccess)
			Debug().LogError("VulkanDevice::CreateTexture: Could not create image view.");

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

		const size_t numAttachments = info.Attachments.size() + (info.HasDepth ? 1 : 0) + (info.HasStencil ? 1 : 0);
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

		size_t depthIndex = 0, stencilIndex = 0;
		if (info.HasDepth)
		{
			depthIndex = textureCounter;
			renderTexture.m_Textures[depthIndex] = CreateTexture({ info.Width, info.Height, PixelFormat::PF_Depth, PixelFormat::PF_Depth32, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Depth, sampler });
			renderTexture.m_AttachmentNames[depthIndex] = "Depth";
			++textureCounter;
		}

		if (info.HasStencil)
		{
			stencilIndex = textureCounter;
			renderTexture.m_Textures[stencilIndex] = CreateTexture({ info.Width, info.Height, PixelFormat::PF_Stencil, PixelFormat::PF_R8Uint, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Stencil, sampler });
			renderTexture.m_AttachmentNames[stencilIndex] = "Stencil";
			++textureCounter;
		}

		std::vector<vk::ImageView> attachments(renderTexture.m_Textures.size());
		for (size_t i = 0; i < renderTexture.m_Textures.size(); ++i)
		{
			TextureHandle textureHandle = renderTexture.m_Textures[i];
			VK_Texture* texture = m_Textures.Find(textureHandle);
			attachments[i] = texture->m_VKImageView;
		}

		if (info.HasDepth)
		{
			VK_Texture* texture = m_Textures.Find(renderTexture.m_Textures[depthIndex]);
			attachments[depthIndex] = texture->m_VKImageView;
		}
		if (info.HasStencil)
		{
			VK_Texture* texture = m_Textures.Find(renderTexture.m_Textures[stencilIndex]);
			attachments[stencilIndex] = texture->m_VKImageView;
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
		attachments.resize(attachmentCount +
			info.RenderTextureInfo.HasDepth || info.RenderTextureInfo.HasStencil ? 1 : 0);
		attachmentColorRefs.resize(attachmentCount);
		for (size_t i = 0; i < attachmentCount; ++i)
		{
			const Attachment& attachment = info.RenderTextureInfo.Attachments[i];

			const vk::Format format = VKConverter::GetVulkanFormat(attachment.Format);

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

		if (info.RenderTextureInfo.HasDepth || info.RenderTextureInfo.HasStencil)
		{
			attachments[attachmentCount] = vk::AttachmentDescription()
				.setFormat(vk::Format::eD24UnormS8Uint)
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

	PipelineHandle VulkanDevice::CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline, size_t stride, const std::vector<AttributeType>& attributeTypes)
	{
		PipelineManager& pipelines = m_pModule->GetEngine()->GetPipelineManager();

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
		{
			pipeline.m_Shaders[i] = CreateShader(pPipeline->Shader(pipelines, i), pPipeline->GetShaderType(pipelines, i), "main");
		}

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(pPipeline->ShaderCount());
		for (size_t i = 0; i < shaderStages.size(); ++i)
		{
			VK_Shader* shader = m_Shaders.Find(pipeline.m_Shaders[i]);
			shaderStages[i] = vk::PipelineShaderStageCreateInfo()
				.setStage(shader->m_VKStage)
				.setModule(shader->m_VKModule)
				.setPName(shader->m_Function.data());
		}

		// Create descriptor set layout
		//m_DescriptorSetLayouts.resize(m_DescriptorSetLayoutInfos.size());
		//
		//for (size_t i = 0; i < m_DescriptorSetLayoutInfos.size(); ++i)
		//{
		//	if (m_LogicalDevice.createDescriptorSetLayout(&m_DescriptorSetLayoutInfos[i], nullptr, &m_DescriptorSetLayouts[i]) != vk::Result::eSuccess)
		//		throw std::runtime_error("Failed to create descriptor set layout!");
		//}

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
			.setFrontFace(vk::FrontFace::eCounterClockwise)
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
		vk::PipelineColorBlendAttachmentState colorBlendAttachmentCreateInfo = vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setBlendEnable(VK_FALSE)
			.setSrcColorBlendFactor(vk::BlendFactor::eOne)
			.setDstColorBlendFactor(vk::BlendFactor::eZero)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd);

		vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(VK_FALSE)
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachmentCount(1)
			.setPAttachments(&colorBlendAttachmentCreateInfo)
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
			.setSetLayoutCount(static_cast<uint32_t>(0))
			.setPSetLayouts(nullptr)
			.setPushConstantRangeCount(0)
			.setPPushConstantRanges(nullptr);

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

		m_LogicalDevice.destroyPipeline(pipeline->m_VKPipeline);
		m_LogicalDevice.destroyPipelineLayout(pipeline->m_VKLayout);
		m_Pipelines.Erase(handle);

		std::stringstream str;
		str << "VulkanDevice: Pipeline " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}
}
