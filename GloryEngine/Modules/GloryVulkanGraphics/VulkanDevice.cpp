#include "VulkanDevice.h"
#include "VulkanGraphicsModule.h"
#include "VulkanStructsConverter.h"
#include "DescriptorAllocator.h"

#include <Debug.h>
#include <Engine.h>
#include <Window.h>

#include <PipelineData.h>
#include <ImageData.h>
#include <MeshData.h>
#include <TextureData.h>
#include <FileData.h>
#include <EngineProfiler.h>

namespace Glory
{
	constexpr size_t ShaderTypeFlagsCount = 6;
	constexpr vk::ShaderStageFlagBits ShaderTypeFlags[ShaderTypeFlagsCount] = {
		vk::ShaderStageFlagBits::eVertex,
		vk::ShaderStageFlagBits::eFragment,
		vk::ShaderStageFlagBits::eGeometry,
		vk::ShaderStageFlagBits::eTessellationControl,
		vk::ShaderStageFlagBits::eTessellationEvaluation,
		vk::ShaderStageFlagBits::eCompute,
	};

	vk::ShaderStageFlags GetShaderStageFlags(const ShaderTypeFlag& shaderTypeFlags)
	{
		vk::ShaderStageFlags result = vk::ShaderStageFlagBits(0);
		for (size_t i = 0; i < ShaderTypeFlagsCount; ++i)
		{
			if (!(shaderTypeFlags & ShaderTypeFlag(1 << i))) continue;
			result |= ShaderTypeFlags[i];
		}
		return result;
	}

	VulkanDevice::VulkanDevice(VulkanGraphicsModule* pModule, vk::PhysicalDevice physicalDevice):
		GraphicsDevice(pModule), m_VKDevice(physicalDevice), m_DidLastSupportCheckPass(false),
		m_DescriptorAllocator(this), m_CommandBufferAllocator(this)
	{
		m_APIFeatures = APIFeatures::All;
	}

	VulkanDevice::~VulkanDevice()
	{
		m_LogicalDevice.waitIdle();
		m_Semaphores.FreeAll(std::bind(&VulkanDevice::FreeSemaphore, this, std::placeholders::_1));
		m_Swapchains.FreeAll(std::bind(&VulkanDevice::FreeSwapchain, this, std::placeholders::_1));
		m_Pipelines.FreeAll(std::bind(&VulkanDevice::FreePipeline, this, std::placeholders::_1));
		m_Shaders.FreeAll(std::bind(&VulkanDevice::FreeShader, this, std::placeholders::_1));
		m_RenderPasses.FreeAll(std::bind(&VulkanDevice::FreeRenderPass, this, std::placeholders::_1));
		m_RenderTextures.FreeAll(std::bind(&VulkanDevice::FreeRenderTexture, this, std::placeholders::_1));
		m_Textures.FreeAll(std::bind(&VulkanDevice::FreeTexture, this, std::placeholders::_1));
		m_Meshes.FreeAll(std::bind(&VulkanDevice::FreeMesh, this, std::placeholders::_1));
		m_Buffers.FreeAll(std::bind(&VulkanDevice::FreeBuffer, this, std::placeholders::_1));
		m_DescriptorSets.Clear();
		m_DescriptorAllocator.ResetPools();
		m_DescriptorSetLayouts.FreeAll(std::bind(&VulkanDevice::FreeDescriptorSetLayout, this, std::placeholders::_1));

		m_LogicalDevice.destroyCommandPool(m_GraphicsCommandPool);
		m_CommandBuffers.clear();
		m_FreeCommandBuffers.clear();

		m_CommandBufferAllocator.Cleanup();

		for (auto fence : m_FreeFences)
			m_LogicalDevice.destroyFence(fence);
		m_FreeFences.clear();
		for (auto& iter : m_CachedSamplers)
			m_LogicalDevice.destroySampler(iter.second);
		m_CachedSamplers.clear();
		m_CachedDescriptorSetLayouts.clear();

		m_LogicalDevice.destroy();
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
			if (m_GraphicsAndComputeFamily.has_value() && m_PresentFamily.has_value()) break;
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR((VkPhysicalDevice)m_VKDevice, i, GraphicsModule()->GetCSurface(), &presentSupport);
			if (!m_GraphicsAndComputeFamily.has_value() && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
				m_GraphicsAndComputeFamily = i;
			if (!m_PresentFamily.has_value() && presentSupport)
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

		if (!m_GraphicsAndComputeFamily.has_value() || !m_PresentFamily.has_value()) return;

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
		std::set<uint32_t> uniqueQueueFamilies = { m_GraphicsAndComputeFamily.value(), m_PresentFamily.value() };

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
		vk::PhysicalDeviceRobustness2FeaturesKHR robustnessFeatures = vk::PhysicalDeviceRobustness2FeaturesKHR();
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
		vk12Features.pNext = &robustnessFeatures;
		robustnessFeatures.nullDescriptor = VK_FALSE;

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
		m_GraphicsAndComputeQueue = m_LogicalDevice.getQueue(m_GraphicsAndComputeFamily.value(), 0);
		m_PresentQueue = m_LogicalDevice.getQueue(m_PresentFamily.value(), 0);

		CreateGraphicsCommandPool();
		AllocateFreeFences(10);

		//m_CommandBufferAllocator
	}

	void VulkanDevice::AllocateFreeFences(size_t numFences)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::AllocateFreeFences" };
		const size_t firstNewFenceIndex = m_FreeFences.size();
		m_FreeFences.resize(m_FreeFences.size() + numFences);
		for (size_t i = firstNewFenceIndex; i < m_FreeFences.size(); ++i)
		{
			vk::FenceCreateInfo fenceCreateInfo = vk::FenceCreateInfo()
				.setFlags((vk::FenceCreateFlagBits)0);

			if (m_LogicalDevice.createFence(&fenceCreateInfo, nullptr, &m_FreeFences[i]) != vk::Result::eSuccess)
				Debug().LogError("VulkanDevice::AllocateFreeFences: Failed to create fence.");
		}
	}

	void VulkanDevice::AllocateFreeCommandBuffers(size_t numBuffers)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::AllocateFreeCommandBuffers" };
		const size_t firstNewCommandBufferIndex = m_FreeCommandBuffers.size();
		m_FreeCommandBuffers.resize(m_FreeCommandBuffers.size() + numBuffers);
		m_CommandBufferAllocator.Allocate(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			numBuffers, &m_FreeCommandBuffers[firstNewCommandBufferIndex]);
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
			.setQueueFamilyIndex(m_GraphicsAndComputeFamily.value())
			.setFlags((vk::CommandPoolCreateFlags)0);

		m_GraphicsCommandPool = m_LogicalDevice.createCommandPool(commandPoolCreateInfo);
		if (m_GraphicsCommandPool == nullptr)
			throw std::runtime_error("failed to create command pool!");
	}

	vk::CommandPool VulkanDevice::GetGraphicsCommandPool()
	{
		return m_GraphicsCommandPool;
	}

	vk::ImageView VulkanDevice::GetVKImageView(TextureHandle texture)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::GetVKImageView" };
		VK_Texture* vkTexture = m_Textures.Find(texture);
		if (!vkTexture)
		{
			Debug().LogError("VulkanDevice::GetVKImageView: Invalid texture handle.");
			return nullptr;
		}
		return vkTexture->m_VKImageView;
	}

	vk::Sampler VulkanDevice::GetVKSampler(TextureHandle texture)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::GetVKSampler" };
		VK_Texture* vkTexture = m_Textures.Find(texture);
		if (!vkTexture)
		{
			Debug().LogError("VulkanDevice::GetVKSampler: Invalid texture handle.");
			return nullptr;
		}
		return vkTexture->m_VKSampler;
	}

	void VulkanDevice::DisableViewportInversion()
	{
		m_InvertViewport = false;
	}

	CommandBufferHandle VulkanDevice::CreateCommandBuffer()
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateCommandBuffer" };
		CommandBufferHandle handle;
		this->GetNewResetableCommandBuffer(handle);
		return handle;
	}

	CommandBufferHandle VulkanDevice::Begin()
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::Begin" };
		CommandBufferHandle handle;
		VK_CommandBuffer& commandBuffer = m_CommandBuffers.emplace(handle, VK_CommandBuffer()).first->second;
		m_CommandBufferAllocator.Allocate(vk::CommandPoolCreateFlags(0), 1, &commandBuffer.m_VKCommandBuffer);
		commandBuffer.m_Resetable = false;

		if (m_FreeFences.empty())
			AllocateFreeFences(10);

		commandBuffer.m_VKFence = m_FreeFences.back();
		m_FreeFences.erase(--m_FreeFences.end());
		if (m_LogicalDevice.resetFences(1, &commandBuffer.m_VKFence) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::Begin: Failed to reset fence.");
			return NULL;
		}

		const vk::CommandBufferBeginInfo commandBeginInfo = vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
			.setPInheritanceInfo(nullptr);
		commandBuffer->begin(commandBeginInfo);

		return handle;
	}

	void VulkanDevice::Begin(CommandBufferHandle commandBuffer)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::Begin(commandBuffer)" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::BeginRenderPass: Invalid command buffer handle.");
			return;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;

		vk::CommandBufferBeginInfo commandBeginInfo = vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
			.setPInheritanceInfo(nullptr);
		vkCommandBuffer->begin(commandBeginInfo);
	}

	void VulkanDevice::BeginRenderPass(CommandBufferHandle commandBuffer, RenderPassHandle renderPass)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::BeginRenderPass" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::BeginRenderPass: Invalid command buffer handle.");
			return;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;

		VK_RenderPass* vkRenderPass = m_RenderPasses.Find(renderPass);
		if (!renderPass)
		{
			Debug().LogError("VulkanDevice::BeginRenderPass: Invalid render pass handle.");
			return;
		}
		VK_RenderTexture* vkRenderTexture = m_RenderTextures.Find(vkRenderPass->m_RenderTexture);
		if (!vkRenderTexture)
		{
			Debug().LogError("VulkanDevice::BeginRenderPass: Render pass has an invalid render texture handle.");
			return;
		}

		// Start a render pass
        const vk::Rect2D renderArea = vk::Rect2D()
            .setOffset(vk::Offset2D(0, 0))
            .setExtent(vk::Extent2D(vkRenderTexture->m_Info.Width, vkRenderTexture->m_Info.Height));

        std::vector<vk::ClearValue> clearColors = std::vector<vk::ClearValue>(vkRenderTexture->m_Textures.size());
        for (size_t i = 0; i < clearColors.size(); ++i)
            clearColors[i].setColor(vkRenderPass->m_ClearColor);

		if (vkRenderTexture->m_HasDepthOrStencil)
			clearColors.back().setDepthStencil(vkRenderPass->m_DepthStencilClear);

        const vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
            .setRenderPass(vkRenderPass->m_VKRenderPass)
            .setFramebuffer(vkRenderTexture->m_VKFramebuffer)
            .setRenderArea(renderArea)
            .setClearValueCount(static_cast<uint32_t>(clearColors.size()))
            .setPClearValues(clearColors.data());

        vkCommandBuffer->beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
	}

	void VulkanDevice::BeginPipeline(CommandBufferHandle commandBuffer, PipelineHandle pipeline)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::BeginPipeline" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::BeginPipeline: Invalid command buffer handle.");
			return;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;

		VK_Pipeline* vkPipeline = m_Pipelines.Find(pipeline);
		if (!pipeline)
		{
			Debug().LogError("VulkanDevice::BeginPipeline: Invalid pipeline handle.");
			return;
		}
		vkCommandBuffer->bindPipeline(vkPipeline->m_VKBindPoint, vkPipeline->m_VKPipeline);
	}

	void VulkanDevice::End(CommandBufferHandle commandBuffer)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::End" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::End: Invalid command buffer handle.");
			return;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;
		vkCommandBuffer->end();
	}

	void VulkanDevice::EndRenderPass(CommandBufferHandle commandBuffer)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::EndRenderPass" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::EndRenderPass: Invalid command buffer handle.");
			return;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;
		vkCommandBuffer->endRenderPass();
	}

	void VulkanDevice::EndPipeline(CommandBufferHandle)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::EndPipeline" };
	}

	void VulkanDevice::BindDescriptorSets(CommandBufferHandle commandBuffer, PipelineHandle pipeline,
		const std::vector<DescriptorSetHandle>& sets, uint32_t firstSet)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::BindDescriptorSets" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::BindDescriptorSets: Invalid command buffer handle.");
			return;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;

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
		vkCommandBuffer->bindDescriptorSets(vkPipeline->m_VKBindPoint, vkPipeline->m_VKLayout,
			firstSet, setsToBind.size(), setsToBind.data(), 0, nullptr);
	}

	void VulkanDevice::PushConstants(CommandBufferHandle commandBuffer, PipelineHandle pipeline, uint32_t offset, uint32_t size, const void* data, ShaderTypeFlag shaderStages)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::PushConstants" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::PushConstants: Invalid command buffer handle.");
			return;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;

		VK_Pipeline* vkPipeline = m_Pipelines.Find(pipeline);
		if (!vkPipeline)
		{
			Debug().LogError("VulkanDevice::PushConstants: Invalid pipeline handle.");
			return;
		}
		const vk::ShaderStageFlags stageFlags = GetShaderStageFlags(shaderStages);
		vkCommandBuffer->pushConstants(vkPipeline->m_VKLayout, stageFlags, offset, size, data);
	}

	void VulkanDevice::DrawMesh(CommandBufferHandle commandBuffer, MeshHandle handle)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::DrawMesh" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::DrawMesh: Invalid command buffer handle.");
			return;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;

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
		vkCommandBuffer->bindVertexBuffers(0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
		if (indexBuffer)
			vkCommandBuffer->bindIndexBuffer(indexBuffer->m_VKBuffer, 0, vk::IndexType::eUint32);

		if (hasIndexBuffer)
			vkCommandBuffer->drawIndexed(mesh->m_IndexCount, 1, 0, 0, 0);
		else
			vkCommandBuffer->draw(mesh->m_VertexCount, 1, 0, 0);
	}

	void VulkanDevice::Dispatch(CommandBufferHandle commandBuffer, uint32_t x, uint32_t y, uint32_t z)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::Dispatch" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::Dispatch: Invalid command buffer handle.");
			return;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;
		vkCommandBuffer->dispatch(x, y, z);
	}

	void VulkanDevice::Commit(CommandBufferHandle commandBuffer, const std::vector<SemaphoreHandle>& waitSemaphores,
		const std::vector<SemaphoreHandle>& signalSemaphores)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::Commit" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::Commit: Invalid command buffer handle.");
			return;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;
		vk::Fence vkFence = vkCommandBuffer.m_VKFence;

		if (m_LogicalDevice.resetFences(1, &vkFence) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::Wait: Failed to reset fence.");
			return;
		}

		/* Submit command buffer */
		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		std::vector<vk::Semaphore> vkWaitSemaphores(waitSemaphores.size());
		std::vector<vk::Semaphore> vkSignalSemaphores(signalSemaphores.size());
		for (size_t i = 0; i < waitSemaphores.size(); ++i)
		{
			VK_Semaphore* vkSemaphore = m_Semaphores.Find(waitSemaphores[i]);
			if (!vkSemaphore)
			{
				Debug().LogError("VulkanDevice::Commit: Invalid semaphore handle for waiting.");
				return;
			}
			vkWaitSemaphores[i] = vkSemaphore->m_VKSemaphore;
		}
		for (size_t i = 0; i < signalSemaphores.size(); ++i)
		{
			VK_Semaphore* vkSemaphore = m_Semaphores.Find(signalSemaphores[i]);
			if (!vkSemaphore)
			{
				Debug().LogError("VulkanDevice::Commit: Invalid semaphore handle for signaling.");
				return;
			}
			vkSignalSemaphores[i] = vkSemaphore->m_VKSemaphore;
		}

		vk::SubmitInfo submitInfo = vk::SubmitInfo()
			.setWaitSemaphoreCount(static_cast<uint32_t>(vkWaitSemaphores.size()))
			.setPWaitSemaphores(vkWaitSemaphores.data())
			.setPWaitDstStageMask(waitStages)
			.setCommandBufferCount(1)
			.setPCommandBuffers(&vkCommandBuffer.m_VKCommandBuffer)
			.setSignalSemaphoreCount(static_cast<uint32_t>(vkSignalSemaphores.size()))
			.setPSignalSemaphores(vkSignalSemaphores.data());

		if (m_GraphicsAndComputeQueue.submit(1, &submitInfo, vkFence) != vk::Result::eSuccess)
			throw std::runtime_error("failed to submit draw command buffer!");
	}

	GraphicsDevice::WaitResult VulkanDevice::Wait(CommandBufferHandle commandBuffer, uint64_t timeout)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::Wait" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::Wait: Invalid command buffer handle.");
			return WR_Fail;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;
		vk::Fence vkFence = vkCommandBuffer.m_VKFence;

		const vk::Result waitState = m_LogicalDevice.waitForFences(1, &vkFence, VK_TRUE, UINT64_MAX);
		if (int32_t(waitState) < 0)
		{
			Debug().LogError("VulkanDevice::Wait: Failed to wait for fence.");
			return WR_Fail;
		}

		switch (waitState)
		{
		case vk::Result::eSuccess:
			return WR_Success;
		case vk::Result::eTimeout:
			return WR_Timeout;
		}

		return WR_Fail;
	}

	void VulkanDevice::Release(CommandBufferHandle commandBuffer)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::Release" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::Wait: Invalid command buffer handle.");
			return;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;
		vk::Fence vkFence = vkCommandBuffer.m_VKFence;

		m_FreeFences.push_back(vkFence);
		vkCommandBuffer.m_VKFence = nullptr;

		if (!vkCommandBuffer.m_Resetable)
		{
			m_CommandBuffers.erase(commandBuffer);
			return;
		}

		vkCommandBuffer->reset();
		m_FreeCommandBuffers.push_back(vkCommandBuffer.m_VKCommandBuffer);
		m_CommandBuffers.erase(commandBuffer);
	}

	void VulkanDevice::Reset(CommandBufferHandle commandBuffer)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::Reset" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::Reset: Invalid command buffer handle.");
			return;
		}
		VK_CommandBuffer& vkCommandBuffer = iter->second;
		vk::Fence vkFence = vkCommandBuffer.m_VKFence;

		if (!vkCommandBuffer.m_Resetable)
		{
			Debug().LogError("VulkanDevice::Reset: Trying to reset a non-resetable command buffer.");
			return;
		}

		vkCommandBuffer->reset();
		if (m_LogicalDevice.resetFences(1, &vkFence) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::Reset: Failed to reset fence.");
			return;
		}
	}

	void VulkanDevice::SetViewport(CommandBufferHandle commandBuffer, float x, float y, float width, float height, float minDepth, float maxDepth)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::SetViewport" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::SetViewport: Invalid command buffer handle.");
			return;
		}
		const VK_CommandBuffer& vkCommandBuffer = iter->second;
		const vk::Viewport viewport = vk::Viewport()
			.setX(x).setY(m_InvertViewport ? height - y : y).setHeight(m_InvertViewport ? -height : height).setWidth(width)
			.setMinDepth(minDepth).setMaxDepth(maxDepth);
		vkCommandBuffer->setViewport(0, 1, &viewport);
	}

	void VulkanDevice::SetScissor(CommandBufferHandle commandBuffer, int x, int y, uint32_t width, uint32_t height)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::SetScissor" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::SetScissor: Invalid command buffer handle.");
			return;
		}
		const VK_CommandBuffer& vkCommandBuffer = iter->second;
		const vk::Rect2D scissor = vk::Rect2D()
			.setOffset({ x,y })
			.setExtent({ width, height });
		vkCommandBuffer->setScissor(0, 1, &scissor);
	}

	void VulkanDevice::PipelineBarrier(CommandBufferHandle commandBuffer, const std::vector<BufferHandle>& buffers,
		const std::vector<TextureHandle>& textures, PipelineStageFlagBits srcStage, PipelineStageFlagBits dstStage)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::PipelineBarrier" };
		auto iter = m_CommandBuffers.find(commandBuffer);
		if (iter == m_CommandBuffers.end())
		{
			Debug().LogError("VulkanDevice::SetViewport: Invalid command buffer handle.");
			return;
		}

		std::vector<vk::BufferMemoryBarrier> bufferBarriers(buffers.size());
		std::vector<vk::ImageMemoryBarrier> imageBarriers(textures.size());
		for (size_t i = 0; i < buffers.size(); ++i)
		{
			VK_Buffer* vkBuffer = m_Buffers.Find(buffers[i]);
			bufferBarriers[i] = vk::BufferMemoryBarrier();
			bufferBarriers[i].buffer = vkBuffer->m_VKBuffer;
			bufferBarriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarriers[i].offset = 0;
			bufferBarriers[i].size = vkBuffer->m_Size;
			//bufferBarriers[i].srcAccessMask
			//bufferBarriers[i].dstAccessMask
		}

		for (size_t i = 0; i < textures.size(); ++i)
		{
			VK_Texture* vkTexture = m_Textures.Find(textures[i]);
			imageBarriers[i] = vk::ImageMemoryBarrier();
			imageBarriers[i].image = vkTexture->m_VKImage;
			imageBarriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarriers[i].oldLayout = vkTexture->m_VKFinalLayout;
			imageBarriers[i].newLayout = vkTexture->m_VKFinalLayout;
			imageBarriers[i].subresourceRange.aspectMask = vkTexture->m_VKAspect;
			imageBarriers[i].subresourceRange.levelCount = 1;
			imageBarriers[i].subresourceRange.layerCount = 1;
			imageBarriers[i].subresourceRange.baseArrayLayer = 0;
			imageBarriers[i].subresourceRange.baseMipLevel = 0;
			//imageBarriers[i].srcAccessMask
			//imageBarriers[i].dstAccessMask
		}

		const VK_CommandBuffer& vkCommandBuffer = iter->second;
		vkCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits(srcStage), vk::PipelineStageFlagBits(dstStage),
			(vk::DependencyFlags)0, 0, nullptr, static_cast<uint32_t>(bufferBarriers.size()), bufferBarriers.data(),
			static_cast<uint32_t>(imageBarriers.size()), imageBarriers.data()
		);
	}

	GraphicsDevice::SwapchainResult VulkanDevice::AqcuireNextSwapchainImage(SwapchainHandle swapchain, uint32_t* imageIndex,
		SemaphoreHandle signalSemaphore)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::AqcuireNextSwapchainImage" };
		VK_Swapchain* vkSwapchain = m_Swapchains.Find(swapchain);
		if (!vkSwapchain)
		{
			Debug().LogError("VulkanDevice::AqcuireNextSwapchainImage: Invalid swap chain handle.");
			return GraphicsDevice::SwapchainResult::S_Error;
		}

		VK_Semaphore* vkSemaphore = m_Semaphores.Find(signalSemaphore);
		vk::Semaphore semaphore = vkSemaphore ? vkSemaphore->m_VKSemaphore : nullptr;

		const vk::Result result = m_LogicalDevice.acquireNextImageKHR(vkSwapchain->m_VKSwapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, imageIndex);
		switch (result)
		{
		case vk::Result::eSuccess:
			return GraphicsDevice::SwapchainResult::S_Success;
		case vk::Result::eSuboptimalKHR:
		case vk::Result::eErrorOutOfDateKHR:
			return GraphicsDevice::SwapchainResult::S_OutOfDate;
		}

		Debug().LogError("VulkanDevice::AqcuireNextSwapchainImage: Failed to acquire image.");
		return GraphicsDevice::SwapchainResult::S_Error;
	}

	GraphicsDevice::SwapchainResult VulkanDevice::Present(SwapchainHandle swapchain, uint32_t imageIndex, const std::vector<SemaphoreHandle>& waitSemaphores)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::Present" };
		VK_Swapchain* vkSwapchain = m_Swapchains.Find(swapchain);
		if (!vkSwapchain)
		{
			Debug().LogError("VulkanDevice::Present: Invalid swap chain handle.");
			return GraphicsDevice::SwapchainResult::S_Error;
		}

		std::vector<vk::Semaphore> vkWaitSemaphores(waitSemaphores.size());
		for (size_t i = 0; i < waitSemaphores.size(); ++i)
		{
			VK_Semaphore* vkSemaphore = m_Semaphores.Find(waitSemaphores[i]);
			if (!vkSemaphore)
			{
				Debug().LogError("VulkanDevice::Present: Invalid semaphore handle for waiting.");
				return GraphicsDevice::SwapchainResult::S_Error;
			}
			vkWaitSemaphores[i] = vkSemaphore->m_VKSemaphore;
		}

		vk::PresentInfoKHR presentInfo{};
		presentInfo.waitSemaphoreCount = static_cast<uint32_t>(vkWaitSemaphores.size());
		presentInfo.pWaitSemaphores = vkWaitSemaphores.data();

		vk::SwapchainKHR swapchains[] = { vkSwapchain->m_VKSwapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		const vk::Result result = m_PresentQueue.presentKHR(presentInfo);
		switch (result)
		{
		case vk::Result::eSuccess:
			return GraphicsDevice::SwapchainResult::S_Success;
		case vk::Result::eSuboptimalKHR:
		case vk::Result::eErrorOutOfDateKHR:
			return GraphicsDevice::SwapchainResult::S_OutOfDate;
		}

		Debug().LogError("VulkanDevice::Present: Failed to present image.");
		return GraphicsDevice::SwapchainResult::S_Error;
	}

	void VulkanDevice::WaitIdle()
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::WaitIdle" };
		m_LogicalDevice.waitIdle();
	}

	vk::BufferUsageFlags GetBufferUsageFlags(BufferType bufferType, BufferFlags flags)
	{
		vk::BufferUsageFlags usageFlags;

		switch (bufferType)
		{
		case Glory::BT_TransferRead:
			usageFlags |= vk::BufferUsageFlagBits::eTransferSrc;
			break;
		case Glory::BT_TransferWrite:
			usageFlags |= vk::BufferUsageFlagBits::eTransferDst;
			break;
		case Glory::BT_Vertex:
			usageFlags |= vk::BufferUsageFlagBits::eVertexBuffer;
			break;
		case Glory::BT_Index:
			usageFlags |= vk::BufferUsageFlagBits::eIndexBuffer;
			break;
		case Glory::BT_Storage:
			usageFlags |= vk::BufferUsageFlagBits::eStorageBuffer;
			break;
		case Glory::BT_Uniform:
			usageFlags |= vk::BufferUsageFlagBits::eUniformBuffer;
			break;
		}
		if (flags & BF_CopyDst)
			usageFlags |= vk::BufferUsageFlagBits::eTransferDst;
		return usageFlags;
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

	vk::MemoryPropertyFlags GetBufferMemoryPropertyFlags(BufferFlags flags)
	{
		if (flags == BF_None)
			return vk::MemoryPropertyFlagBits::eDeviceLocal;
		vk::MemoryPropertyFlags result;
		if (flags & BF_Write)
		{
			result |= vk::MemoryPropertyFlagBits::eHostVisible;
			result |= vk::MemoryPropertyFlagBits::eHostCoherent;
		}
		if (flags & BF_Read)
		{
			result |= vk::MemoryPropertyFlagBits::eHostVisible;
			result |= vk::MemoryPropertyFlagBits::eHostCoherent;
			result |= vk::MemoryPropertyFlagBits::eHostCached;
		}
		return result;
	}

	BufferHandle VulkanDevice::CreateBuffer(size_t bufferSize, BufferType type, BufferFlags flags)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateBuffer" };
		BufferHandle handle;
		VK_Buffer& buffer = m_Buffers.Emplace(handle, VK_Buffer());
		buffer.m_Size = bufferSize;
		buffer.m_CPUVisible = (flags & BF_ReadAndWrite) != 0;

		vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo();
		bufferInfo.size = (vk::DeviceSize)buffer.m_Size;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;
		bufferInfo.usage = buffer.m_VKUsage = GetBufferUsageFlags(type, flags);

		vk::Result result = m_LogicalDevice.createBuffer(&bufferInfo, nullptr, &buffer.m_VKBuffer);
		if (result != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateBuffer: Failed to create buffer.");
			m_Buffers.Erase(handle);
			return NULL;
		}

		vk::MemoryRequirements memRequirements;
		m_LogicalDevice.getBufferMemoryRequirements(buffer.m_VKBuffer, &memRequirements);

		const uint32_t typeFilter = memRequirements.memoryTypeBits;
		const vk::MemoryPropertyFlags properties = GetBufferMemoryPropertyFlags(flags);
		const uint32_t memoryIndex = GetSupportedMemoryIndex(typeFilter, properties);

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
		ProfileSample s{ &Profiler(), "VulkanDevice::AssignBuffer" };
		if (!data) return;

		VK_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("VulkanDevice::AssignBuffer: Invalid buffer handle.");
			return;
		}
		AssignBuffer(handle, data, 0, buffer->m_Size);
	}

	void VulkanDevice::AssignBuffer(BufferHandle handle, const void* data, uint32_t size)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::AssignBuffer(size)" };
		if (!data) return;
		AssignBuffer(handle, data, 0, size);
	}

	void VulkanDevice::AssignBuffer(BufferHandle handle, const void* data, uint32_t offset, uint32_t size)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::AssignBuffer(offset, size)" };
		if (!data) return;

		VK_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("VulkanDevice::FreeBuffer: Invalid buffer handle.");
			return;
		}

		if (!buffer->m_CPUVisible)
		{
			/* We have to assign it using a buffer copy */
			BufferHandle staging = CreateBuffer(size, BufferType::BT_TransferRead, BufferFlags::BF_Write);
			AssignBuffer(staging, data, offset, size);
			VK_Buffer* vkStaging = m_Buffers.Find(staging);
			CopyFromBuffer(buffer->m_VKBuffer, vkStaging->m_VKBuffer, offset, 0, size);
			FreeBuffer(staging);
			return;
		}

		if (!buffer->m_pMappedMemory)
		{
			const vk::Result result = m_LogicalDevice.mapMemory(buffer->m_VKMemory, (vk::DeviceSize)0, (vk::DeviceSize)buffer->m_Size, (vk::MemoryMapFlags)0, &buffer->m_pMappedMemory);
			if (result != vk::Result::eSuccess)
			{
				Debug().LogError("VulkanDevice::CreateBuffer: Failed to map buffer memory.");
				return;
			}
		}

		char* p = (char*)buffer->m_pMappedMemory;
		void* offsettedData = p + offset;
		memcpy(offsettedData, data, size);
	}

	void VulkanDevice::ReadBuffer(BufferHandle handle, void* outData, uint32_t offset, uint32_t size)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::ReadBuffer(offset, size)" };
		if (!outData) return;

		VK_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("VulkanDevice::ReadBuffer: Invalid buffer handle.");
			return;
		}

		if (!buffer->m_CPUVisible)
		{
			Debug().LogError("VulkanDevice::ReadBuffer: Can't read a buffer that is not visible to the CPU.");
			return;
		}

		if (!buffer->m_pMappedMemory)
		{
			const vk::Result result = m_LogicalDevice.mapMemory(buffer->m_VKMemory, (vk::DeviceSize)0, (vk::DeviceSize)buffer->m_Size, (vk::MemoryMapFlags)0, &buffer->m_pMappedMemory);
			if (result != vk::Result::eSuccess)
			{
				Debug().LogError("VulkanDevice::CreateBuffer: Failed to map buffer memory.");
				return;
			}
		}

		char* p = (char*)buffer->m_pMappedMemory;
		void* offsettedData = p + offset;
		std::memcpy(outData, offsettedData, size);
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

	MeshHandle VulkanDevice::CreateMesh(std::vector<BufferHandle>&& buffers, uint32_t vertexCount, uint32_t indexCount,
		uint32_t stride, PrimitiveType primitiveType, const std::vector<AttributeType>& attributeTypes)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateMesh" };
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

	void VulkanDevice::UpdateMesh(MeshHandle mesh, std::vector<BufferHandle>&& buffers, uint32_t vertexCount, uint32_t indexCount)
	{
		VK_Mesh* vkMesh = m_Meshes.Find(mesh);
		if (!vkMesh)
		{
			Debug().LogError("VulkanDevice::UpdateMesh: Invalid mesh handle.");
			return;
		}

		vkMesh->m_VertexCount = vertexCount;
		vkMesh->m_IndexCount = indexCount;
		if (buffers.empty()) return;
		vkMesh->m_Buffers = std::move(buffers);
	}

	TextureHandle VulkanDevice::CreateTexture(TextureData* pTexture)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateTexture" };
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
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateTexture(textureInfo)" };
		TextureHandle handle;
		VK_Texture& texture = m_Textures.Emplace(handle, VK_Texture());

		const uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(textureInfo.m_Width, textureInfo.m_Height)))) + 1;
		const vk::Format format = VKConverter::GetVulkanFormat(textureInfo.m_InternalFormat); //vk::Format::eR8G8B8A8Srgb;
		const vk::ImageType imageType = VKConverter::GetVulkanImageType(textureInfo.m_ImageType);
		vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo();
		imageInfo.imageType = imageType;
		imageInfo.extent.width = textureInfo.m_Width;
		imageInfo.extent.height = textureInfo.m_Height;
		imageInfo.extent.depth = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = VKConverter::GetVulkanImageUsageFlags(textureInfo.m_ImageAspectFlags);
		if (textureInfo.m_SamplingEnabled) imageInfo.usage |= vk::ImageUsageFlagBits::eSampled;
		if (pixels) imageInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.flags = (vk::ImageCreateFlags)0;
		imageInfo.mipLevels = !textureInfo.m_SamplingEnabled ||
			textureInfo.m_SamplerSettings.MipmapMode == Filter::F_None ? 1 : mipLevels;
		if (imageInfo.mipLevels > 1)
			imageInfo.usage |= vk::ImageUsageFlagBits::eTransferSrc;

		vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo();
		EnsureSupportedFormat(imageInfo.format, viewInfo);

		texture.m_VKAspect = VKConverter::GetVulkanImageAspectFlags(textureInfo.m_ImageAspectFlags);

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
		/* Transition image layout */
		if (pixels)
		{
			TransitionImageLayout(texture.m_VKImage, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, texture.m_VKAspect, imageInfo.mipLevels);
			const vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			BufferHandle stagingBuffer = CreateBuffer(memRequirements.size, BufferType::BT_TransferRead, BufferFlags::BF_Write);
			VK_Buffer* vkStagingBuffer = m_Buffers.Find(stagingBuffer);
			AssignBuffer(stagingBuffer, pixels, uint32_t(dataSize));

			CopyFromBuffer(vkStagingBuffer->m_VKBuffer, texture.m_VKImage, texture.m_VKAspect, textureInfo.m_Width, textureInfo.m_Height);

			/* Transtion layout again so it can be sampled */
			if (imageInfo.mipLevels > 1)
				GenerateMipMaps(texture.m_VKImage, textureInfo.m_Width, textureInfo.m_Height, imageInfo.mipLevels);
			else
				TransitionImageLayout(texture.m_VKImage, format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, texture.m_VKAspect, imageInfo.mipLevels);

			FreeBuffer(stagingBuffer);

			texture.m_VKFinalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		/* Create texture image view */
		viewInfo.image = texture.m_VKImage;
		viewInfo.viewType = VKConverter::GetVulkanImageViewType(textureInfo.m_ImageType);
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = texture.m_VKAspect;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (m_LogicalDevice.createImageView(&viewInfo, nullptr, &texture.m_VKImageView) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateTexture: Could not create image view.");
			return NULL;
		}

		if (textureInfo.m_SamplingEnabled)
		{
			auto samplerIter = m_CachedSamplers.find(textureInfo.m_SamplerSettings);
			if (samplerIter == m_CachedSamplers.end())
			{
				auto samplerCreateInfo = VKConverter::GetVulkanSamplerInfo(textureInfo.m_SamplerSettings);

				vk::Sampler newSampler;
				if (m_LogicalDevice.createSampler(&samplerCreateInfo, nullptr, &newSampler) != vk::Result::eSuccess)
				{
					Debug().LogError("VulkanDevice::CreateTexture: Could not create image sampler.");
					return NULL;
				}
				samplerIter = m_CachedSamplers.emplace(textureInfo.m_SamplerSettings, newSampler).first;
			}
			texture.m_VKSampler = samplerIter->second;
		}

		std::stringstream str;
		str << "VulkanDevice: Texture " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	RenderTextureHandle VulkanDevice::CreateRenderTexture(RenderPassHandle renderPass, RenderTextureCreateInfo&& info)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateRenderTexture" };
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
		renderTexture.m_HasDepthOrStencil = info.HasDepth || info.HasStencil;
		renderTexture.m_Info = std::move(info);
		CreateRenderTexture(vkRenderPass->m_VKRenderPass, renderTexture);
		return handle;
	}

	TextureHandle VulkanDevice::GetRenderTextureAttachment(RenderTextureHandle renderTexture, size_t index)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::GetRenderTextureAttachment" };
		VK_RenderTexture* vkRenderTexture = m_RenderTextures.Find(renderTexture);
		if (!vkRenderTexture)
		{
			Debug().LogError("VulkanDevice::GetRenderTextureAttatchment: Invalid render texture handle");
			return NULL;
		}

		if (index >= vkRenderTexture->m_Textures.size())
		{
			Debug().LogError("VulkanDevice::GetRenderTextureAttatchment: Invalid attachment index");
			return NULL;
		}
		return vkRenderTexture->m_Textures[index];
	}

	void VulkanDevice::ResizeRenderTexture(RenderTextureHandle renderTexture, uint32_t width, uint32_t height)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::ResizeRenderTexture" };
		VK_RenderTexture* vkRenderTexture = m_RenderTextures.Find(renderTexture);
		if (!vkRenderTexture)
		{
			Debug().LogError("VulkanDevice::ResizeRenderTexture: Invalid render texture handle");
			return;
		}

		VK_RenderPass* vkRenderPass = m_RenderPasses.Find(vkRenderTexture->m_RenderPass);
		if (!vkRenderPass)
		{
			Debug().LogError("VulkanDevice::ResizeRenderTexture: Invalid render pass handle");
			return;
		}

		vkRenderTexture->m_Info.Width = width;
		vkRenderTexture->m_Info.Height = height;

		for (size_t i = 0; i < vkRenderTexture->m_Textures.size(); ++i)
		{
			FreeTexture(vkRenderTexture->m_Textures[i]);
		}
		vkRenderTexture->m_Textures.clear();
		vkRenderTexture->m_AttachmentNames.clear();
		m_LogicalDevice.destroyFramebuffer(vkRenderTexture->m_VKFramebuffer);
		CreateRenderTexture(vkRenderPass->m_VKRenderPass, *vkRenderTexture);
	}

	vk::AttachmentLoadOp GetLoadOp(RenderPassLoadOp loadOp)
	{
		switch (loadOp)
		{
		case Glory::OP_DontCare:
			return vk::AttachmentLoadOp::eDontCare;
		case Glory::OP_Clear:
			return vk::AttachmentLoadOp::eClear;
		case Glory::OP_Load:
			return vk::AttachmentLoadOp::eLoad;
		default:
			break;
		}
	}

	vk::ImageLayout GetInitialLayout(RenderPassPosition passPosition, VK_Texture* texture, VK_RenderTexture* renderTexture, vk::ImageLayout defaultLayout)
	{
		switch (passPosition)
		{
		case Glory::RP_Start:
			return texture ? texture->m_VKInitialLayout : vk::ImageLayout::eUndefined;
		case Glory::RP_Middle:
		case Glory::RP_Final:
			return renderTexture ? defaultLayout : vk::ImageLayout::eUndefined;
		default:
			return vk::ImageLayout::eUndefined;
		}
	}

	vk::ImageLayout GetFinalLayout(RenderPassPosition passPosition, VK_Texture* texture, bool samplingEnabled, vk::ImageLayout defaultLayout)
	{
		switch (passPosition)
		{
		case Glory::RP_Start:
		case Glory::RP_Middle:
			return defaultLayout;
		case Glory::RP_Final:
			return texture ? texture->m_VKFinalLayout :
				(samplingEnabled ? vk::ImageLayout::eShaderReadOnlyOptimal : defaultLayout);
		default:
			break;
		}
	}

	RenderPassHandle VulkanDevice::CreateRenderPass(RenderPassInfo&& info)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateRenderPass" };

		VK_RenderTexture* renderTexture = m_RenderTextures.Find(info.RenderTexture);
		const RenderTextureCreateInfo& renderTextureInfo = renderTexture ? renderTexture->m_Info : info.RenderTextureInfo;
		if (renderTextureInfo.Width == 0 || renderTextureInfo.Height == 0)
		{
			Debug().LogError("VulkanDevice::CreateRenderPass: Invalid RenderTexture size.");
			return NULL;
		}

		RenderPassHandle handle;
		VK_RenderPass& renderPass = m_RenderPasses.Emplace(handle, VK_RenderPass());
		memcpy(&renderPass.m_ClearColor, (const void*)&info.m_ClearColor, sizeof(float)*4);
		renderPass.m_DepthStencilClear = vk::ClearDepthStencilValue(info.m_DepthClear, info.m_StencilClear);

		std::vector<vk::AttachmentDescription> attachments;
		std::vector<vk::AttachmentReference> attachmentColorRefs;
		vk::AttachmentReference attachmentDepthStencilRef;
		const size_t attachmentCount = renderTextureInfo.Attachments.size();
		attachments.resize(attachmentCount + (
			renderTextureInfo.HasDepth || renderTextureInfo.HasStencil ? 1 : 0));
		attachmentColorRefs.resize(attachmentCount);

		const vk::AttachmentLoadOp loadOp = GetLoadOp(info.m_LoadOp);

		for (size_t i = 0; i < attachmentCount; ++i)
		{
			const Attachment& attachment = renderTextureInfo.Attachments[i];

			VK_Texture* texture = m_Textures.Find(attachment.Texture);
			const vk::Format format = texture && texture->m_VKFormat != vk::Format::eUndefined ? texture->m_VKFormat :
				VKConverter::GetVulkanFormat(attachment.InternalFormat);
			const vk::ImageLayout initialLayout = GetInitialLayout(info.m_Position, texture, renderTexture, vk::ImageLayout::eColorAttachmentOptimal);
			const vk::ImageLayout finalLayout = GetFinalLayout(info.m_Position, texture, attachment.m_SamplingEnabled, vk::ImageLayout::eColorAttachmentOptimal);

			// Create render pass
			attachments[i] = vk::AttachmentDescription()
				.setFormat(format)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(loadOp)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(initialLayout)
				.setFinalLayout(finalLayout);

			attachmentColorRefs[i] = vk::AttachmentReference()
				.setAttachment(i)
				.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
		}

		if (renderTextureInfo.HasDepth && renderTextureInfo.HasStencil)
		{
			const vk::ImageLayout initialLayout = GetInitialLayout(info.m_Position, nullptr, renderTexture,
				vk::ImageLayout::eDepthStencilAttachmentOptimal);
			const vk::ImageLayout finalLayout = GetFinalLayout(info.m_Position, nullptr,
				renderTextureInfo.EnableDepthStencilSampling, vk::ImageLayout::eDepthStencilAttachmentOptimal);

			attachments[attachmentCount] = vk::AttachmentDescription()
				.setFormat(vk::Format::eD32SfloatS8Uint)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(loadOp)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setStencilLoadOp(loadOp)
				.setStencilStoreOp(vk::AttachmentStoreOp::eStore)
				.setInitialLayout(initialLayout)
				.setFinalLayout(finalLayout);

			attachmentDepthStencilRef = vk::AttachmentReference()
				.setAttachment(attachmentCount)
				.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		}
		else if (renderTextureInfo.HasDepth)
		{
			const vk::ImageLayout initialLayout = GetInitialLayout(info.m_Position, nullptr, renderTexture,
				vk::ImageLayout::eDepthAttachmentOptimal);
			const vk::ImageLayout finalLayout = GetFinalLayout(info.m_Position, nullptr,
				renderTextureInfo.EnableDepthStencilSampling, vk::ImageLayout::eDepthAttachmentOptimal);

			attachments[attachmentCount] = vk::AttachmentDescription()
				.setFormat(vk::Format::eD32Sfloat)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(loadOp)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setStencilLoadOp(loadOp)
				.setStencilStoreOp(vk::AttachmentStoreOp::eStore)
				.setInitialLayout(initialLayout)
				.setFinalLayout(finalLayout);

			attachmentDepthStencilRef = vk::AttachmentReference()
				.setAttachment(attachmentCount)
				.setLayout(vk::ImageLayout::eDepthAttachmentOptimal);
		}
		else if (renderTextureInfo.HasStencil)
		{
			const vk::ImageLayout initialLayout = GetInitialLayout(info.m_Position, nullptr, renderTexture,
				vk::ImageLayout::eStencilAttachmentOptimal);
			const vk::ImageLayout finalLayout = GetFinalLayout(info.m_Position, nullptr,
				renderTextureInfo.EnableDepthStencilSampling, vk::ImageLayout::eStencilAttachmentOptimal);

			attachments[attachmentCount] = vk::AttachmentDescription()
				.setFormat(vk::Format::eS8Uint)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(loadOp)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setStencilLoadOp(loadOp)
				.setStencilStoreOp(vk::AttachmentStoreOp::eStore)
				.setInitialLayout(initialLayout)
				.setFinalLayout(finalLayout);

			attachmentDepthStencilRef = vk::AttachmentReference()
				.setAttachment(attachmentCount)
				.setLayout(vk::ImageLayout::eStencilAttachmentOptimal);
		}

		vk::SubpassDescription subPass = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(static_cast<uint32_t>(attachmentColorRefs.size()))
			.setPColorAttachments(attachmentColorRefs.empty() ? nullptr : attachmentColorRefs.data());
		if (renderTextureInfo.HasDepth || renderTextureInfo.HasStencil)
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
			.setPAttachments(attachments.empty() ? nullptr : attachments.data())
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

		renderPass.m_RenderTexture = renderTexture ? info.RenderTexture :
			CreateRenderTexture(handle, std::move(info.RenderTextureInfo));

		if (!renderPass.m_RenderTexture)
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

	RenderTextureHandle VulkanDevice::GetRenderPassRenderTexture(RenderPassHandle renderPass)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::GetRenderPassRenderTexture" };
		VK_RenderPass* vkRenderPass = m_RenderPasses.Find(renderPass);
		if (!vkRenderPass)
		{
			Debug().LogError("VulkanDevice::GetRenderPassRenderTexture: Invalid render pass handle");
			return NULL;
		}
		return vkRenderPass->m_RenderTexture;
	}

	void VulkanDevice::SetRenderPassClear(RenderPassHandle renderPass, const glm::vec4& color, float depth, uint8_t stencil)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::SetRenderPassClear" };
		VK_RenderPass* vkRenderPass = m_RenderPasses.Find(renderPass);
		if (!vkRenderPass)
		{
			Debug().LogError("VulkanDevice::SetRenderPassClear: Invalid render pass handle");
			return;
		}
		memcpy(&vkRenderPass->m_ClearColor, (const void*)&color, sizeof(float)*4);
		vkRenderPass->m_DepthStencilClear = vk::ClearDepthStencilValue(depth, stencil);
	}

	ShaderHandle VulkanDevice::CreateShader(const FileData* pShaderFileData, const ShaderType& shaderType, const std::string& function)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateShader" };
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

	vk::CullModeFlagBits GetVKCullMode(CullFace cullFace)
	{
		switch (cullFace)
		{
		case Glory::CullFace::None:
			return vk::CullModeFlagBits::eNone;
		case Glory::CullFace::Front:
			return vk::CullModeFlagBits::eFront;
		case Glory::CullFace::Back:
			return vk::CullModeFlagBits::eBack;
		case Glory::CullFace::FrontAndBack:
			return vk::CullModeFlagBits::eFrontAndBack;
		default:
			return vk::CullModeFlagBits::eNone;
		}
	}

	PipelineHandle VulkanDevice::CreatePipeline(RenderPassHandle renderPass, PipelineData* pPipeline,
		std::vector<DescriptorSetLayoutHandle>&& descriptorSetLayouts, size_t stride, const std::vector<AttributeType>& attributeTypes)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreatePipeline" };
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
		pipeline.m_VKBindPoint = vk::PipelineBindPoint::eGraphics;

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
			.setY(m_InvertViewport ? (float)vkRenderTexture->m_Info.Height : 0.0f)
			.setWidth((float)vkRenderTexture->m_Info.Width)
			.setHeight((float)vkRenderTexture->m_Info.Height*(m_InvertViewport ? -1.0f : 1.0f))
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f);

		vk::Rect2D scissor = vk::Rect2D()
			.setOffset({ 0,0 })
			.setExtent({ vkRenderTexture->m_Info.Width, vkRenderTexture->m_Info.Height });

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
			.setCullMode(GetVKCullMode(pPipeline->GetCullFace()))
			.setFrontFace(m_InvertViewport ? vk::FrontFace::eCounterClockwise : vk::FrontFace::eClockwise)
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
		vk::DynamicState dynamicStates[] = {
		    vk::DynamicState::eViewport,
		    vk::DynamicState::eScissor,
		    //vk::DynamicState::eLineWidth
		};

		vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
		    .setDynamicStateCount(2)
		    .setPDynamicStates(dynamicStates);

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(static_cast<uint32_t>(vkDescriptorSetLayouts.size()))
			.setPSetLayouts(vkDescriptorSetLayouts.data())
			.setPushConstantRangeCount(static_cast<uint32_t>(pushConstants.size()))
			.setPPushConstantRanges(pushConstants.data());

		pipeline.m_VKLayout = m_LogicalDevice.createPipelineLayout(pipelineLayoutCreateInfo);
		if (pipeline.m_VKLayout == nullptr)
		{
			Debug().LogError("VulkanDevice::CreatePipeline: Failed to create pipeline layout.");
			return NULL;
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
			.setPDynamicState(&dynamicStateCreateInfo)
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

	PipelineHandle VulkanDevice::CreateComputePipeline(PipelineData* pPipeline, std::vector<DescriptorSetLayoutHandle>&& descriptorSetLayouts)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateComputePipeline" };
		PipelineManager& pipelines = m_pModule->GetEngine()->GetPipelineManager();
		std::vector<vk::PushConstantRange> pushConstants;

		if (pPipeline->ShaderCount() > 1 || pPipeline->ShaderCount() == 0 || pPipeline->GetShaderType(pipelines, 0) != ShaderType::ST_Compute)
		{
			Debug().LogError("VulkanDevice::CreateComputePipeline: Pipeline must have only 1 shader and it must be a compute shader.");
			return NULL;
		}

		std::vector<vk::DescriptorSetLayout> vkDescriptorSetLayouts(descriptorSetLayouts.size());
		for (size_t i = 0; i < descriptorSetLayouts.size(); ++i)
		{
			VK_DescriptorSetLayout* vkSetLayout = m_DescriptorSetLayouts.Find(descriptorSetLayouts[i]);
			if (!vkSetLayout)
			{
				Debug().LogError("VulkanDevice::CreateComputePipeline: Invalid descriptor set layout handle.");
				return NULL;
			}
			vkDescriptorSetLayouts[i] = vkSetLayout->m_VKLayout;

			if (vkSetLayout->m_PushConstantRange.size)
				pushConstants.push_back(vkSetLayout->m_PushConstantRange);
		}

		PipelineHandle handle;
		VK_Pipeline& pipeline = m_Pipelines.Emplace(handle, VK_Pipeline());
		pipeline.m_RenderPass = 0;
		pipeline.m_VKBindPoint = vk::PipelineBindPoint::eCompute;

		pipeline.m_Shaders.resize(1);
		pipeline.m_Shaders[0] = CreateShader(pPipeline->Shader(pipelines, 0), ShaderType::ST_Compute, "main");

		VK_Shader* shader = m_Shaders.Find(pipeline.m_Shaders[0]);
		vk::PipelineShaderStageCreateInfo shaderStage = vk::PipelineShaderStageCreateInfo()
			.setStage(shader->m_VKStage)
			.setModule(shader->m_VKModule)
			.setPName(shader->m_Function.data());

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(static_cast<uint32_t>(vkDescriptorSetLayouts.size()))
			.setPSetLayouts(vkDescriptorSetLayouts.data())
			.setPushConstantRangeCount(static_cast<uint32_t>(pushConstants.size()))
			.setPPushConstantRanges(pushConstants.data());

		pipeline.m_VKLayout = m_LogicalDevice.createPipelineLayout(pipelineLayoutCreateInfo);
		if (pipeline.m_VKLayout == nullptr)
		{
			Debug().LogError("VulkanDevice::CreateComputePipeline: Failed to create pipeline layout.");
			return NULL;
		}

		// Create the pipeline
		vk::ComputePipelineCreateInfo pipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setStage(shaderStage)
			.setLayout(pipeline.m_VKLayout);

		if (m_LogicalDevice.createComputePipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline.m_VKPipeline) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateComputePipeline: Failed to create compute pipeline.");
			return NULL;
		}

		std::stringstream str;
		str << "VulkanDevice: Compute pipeline " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	DescriptorSetLayoutHandle VulkanDevice::CreateDescriptorSetLayout(DescriptorSetLayoutInfo&& setLayoutInfo)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateDescriptorSetLayout" };
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
				layoutBindings[i].stageFlags = GetShaderStageFlags(bufferInfo.m_ShaderStages);
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
				layoutBindings[index].stageFlags = GetShaderStageFlags(setLayoutInfo.m_Samplers[i].m_ShaderStages);
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
			setLayout.m_PushConstantRange.stageFlags = GetShaderStageFlags(setLayoutInfo.m_PushConstantRange.m_ShaderStages);
			setLayout.m_BindingIndices = std::move(bindingIndices);
			setLayout.m_DescriptorTypes = std::move(descriptorTypes);
		}
		return iter->second;
	}

	DescriptorSetHandle VulkanDevice::CreateDescriptorSet(DescriptorSetInfo&& setInfo)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateDescriptorSet" };
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
			if (!vkTexture) vkTexture = m_Textures.Find(m_DefaultTexture);

			imageInfos[i].imageLayout = vkTexture ? vkTexture->m_VKFinalLayout : vk::ImageLayout::eUndefined;
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

		m_DescriptorAllocator.Allocate(&set.m_VKDescriptorSet, &set.m_VKDescriptorPool, vkDescriptorSetLayout->m_VKLayout);
		for (size_t i = 0; i < descriptorWrites.size(); ++i)
			descriptorWrites[i].dstSet = set.m_VKDescriptorSet;
		m_LogicalDevice.updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

		return handle;
	}

	void VulkanDevice::UpdateDescriptorSet(DescriptorSetHandle descriptorSet, const DescriptorSetUpdateInfo& setWriteInfo)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::UpdateDescriptorSet" };
		VK_DescriptorSet* vkDescriptorSet = m_DescriptorSets.Find(descriptorSet);
		if (!vkDescriptorSet)
		{
			Debug().LogError("VulkanDevice::UpdateDescriptorSet: Invalid descriptor set handle.");
			return;
		}

		VK_DescriptorSetLayout* vkDescriptorSetLayout = m_DescriptorSetLayouts.Find(vkDescriptorSet->m_Layout);
		if (!vkDescriptorSetLayout)
		{
			Debug().LogError("VulkanDevice::UpdateDescriptorSet: Invalid descriptor set layout handle.");
			return;
		}

		std::vector<vk::WriteDescriptorSet> descriptorWrites(setWriteInfo.m_Buffers.size() + setWriteInfo.m_Samplers.size());
		std::vector<vk::DescriptorBufferInfo> bufferInfos(setWriteInfo.m_Buffers.size());
		std::vector<vk::DescriptorImageInfo> imageInfos(setWriteInfo.m_Samplers.size());
		for (size_t i = 0; i < setWriteInfo.m_Buffers.size(); ++i)
		{
			auto& bufferInfo = setWriteInfo.m_Buffers[i];
			VK_Buffer* vkBuffer = m_Buffers.Find(setWriteInfo.m_Buffers[i].m_BufferHandle);
			if (!vkBuffer)
			{
				Debug().LogError("VulkanDevice::UpdateDescriptorSet: Invalid buffer handle.");
				return;
			}

			bufferInfos[i].buffer = vkBuffer->m_VKBuffer;
			bufferInfos[i].offset = bufferInfo.m_Offset;
			bufferInfos[i].range = bufferInfo.m_Size;

			descriptorWrites[i].dstBinding = vkDescriptorSetLayout->m_BindingIndices[bufferInfo.m_DescriptorIndex];
			descriptorWrites[i].dstArrayElement = 0;
			descriptorWrites[i].descriptorType = vkDescriptorSetLayout->m_DescriptorTypes[bufferInfo.m_DescriptorIndex];
			descriptorWrites[i].descriptorCount = 1;
			descriptorWrites[i].pBufferInfo = &bufferInfos[i];
			descriptorWrites[i].pImageInfo = nullptr;
			descriptorWrites[i].pTexelBufferView = nullptr;
		}

		for (size_t i = 0; i < setWriteInfo.m_Samplers.size(); ++i)
		{
			const size_t index = setWriteInfo.m_Buffers.size() + i;
			auto& samplerInfo = setWriteInfo.m_Samplers[i];
			VK_Texture* vkTexture = m_Textures.Find(setWriteInfo.m_Samplers[i].m_TextureHandle);
			if (!vkTexture) vkTexture = m_Textures.Find(m_DefaultTexture);

			imageInfos[i].imageLayout = vkTexture ? vkTexture->m_VKFinalLayout : vk::ImageLayout::eUndefined;
			imageInfos[i].imageView = vkTexture ? vkTexture->m_VKImageView : nullptr;
			imageInfos[i].sampler = vkTexture ? vkTexture->m_VKSampler : nullptr;

			descriptorWrites[index].dstBinding = vkDescriptorSetLayout->m_BindingIndices[samplerInfo.m_DescriptorIndex];
			descriptorWrites[index].dstArrayElement = 0;
			descriptorWrites[index].descriptorType = vkDescriptorSetLayout->m_DescriptorTypes[samplerInfo.m_DescriptorIndex];
			descriptorWrites[index].descriptorCount = 1;
			descriptorWrites[index].pImageInfo = &imageInfos[i];
		}

		for (size_t i = 0; i < descriptorWrites.size(); ++i)
			descriptorWrites[i].dstSet = vkDescriptorSet->m_VKDescriptorSet;
		m_LogicalDevice.updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}

	std::vector<vk::PresentModeKHR> VSyncPresentModes = { vk::PresentModeKHR::eFifo };
	std::vector<vk::PresentModeKHR> InfiniteFPSPresentModes = { vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate, vk::PresentModeKHR::eFifo };

	uint32_t GetMinImageCountFromPresentMode(vk::PresentModeKHR presentMode)
	{
		if (presentMode == vk::PresentModeKHR::eMailbox)
			return 3u;
		if (presentMode == vk::PresentModeKHR::eFifo || presentMode == vk::PresentModeKHR::eFifoRelaxed)
			return 2u;
		if (presentMode == vk::PresentModeKHR::eImmediate)
			return 1u;
		return 1u;
	}

	SwapchainHandle VulkanDevice::CreateSwapchain(Window* pWindow, bool vsync, uint32_t minImageCount)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateSwapchain" };
		// Check swapchain support
		vk::SurfaceCapabilitiesKHR swapchainCapabilities;
		vk::SurfaceKHR surface;
		pWindow->GetVulkanSurface(GraphicsModule()->GetCInstance(), &surface);
		vk::Result vkError = m_VKDevice.getSurfaceCapabilitiesKHR(surface, &swapchainCapabilities);
		if (vkError != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateSwapchain: Failed to get surface capabilities: Vulkan Error Code: " + std::to_string((uint32_t)vkError));
			return NULL;
		}

		std::vector<vk::SurfaceFormatKHR> swapchainFormats = m_VKDevice.getSurfaceFormatsKHR(surface);

		if (swapchainFormats.empty())
		{
			Debug().LogError("VulkanDevice::CreateSwapchain: Surface does not support any formats");
			return NULL;
		}

		int width, height;
		pWindow->GetDrawableSize(&width, &height);
		const glm::uvec2 resolution = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		SwapchainHandle handle;
		VK_Swapchain& swapchain = m_Swapchains.Emplace(handle, VK_Swapchain());
		swapchain.m_pWindow = pWindow;
		swapchain.m_Vsync = vsync;
		if (!CreateSwapchain(swapchain, swapchainCapabilities, surface, resolution, vsync, minImageCount))
		{
			m_Swapchains.Erase(handle);
			return NULL;
		}

		std::stringstream str;
		str << "VulkanDevice: Swap chain " << handle << " created.";
		Debug().LogInfo(str.str());

		return handle;
	}

	uint32_t VulkanDevice::GetSwapchainImageCount(SwapchainHandle swapchain)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::GetSwapchainImageCount" };
		VK_Swapchain* vkSwapchain = m_Swapchains.Find(swapchain);
		if (!vkSwapchain)
		{
			Debug().LogError("VulkanDevice::GetSwapchainImageCount: Invalid swap chain handle.");
			return 0;
		}
		return vkSwapchain->m_Textures.size();
	}

	TextureHandle VulkanDevice::GetSwapchainImage(SwapchainHandle swapchain, uint32_t imageIndex)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::GetSwapchainImage" };
		VK_Swapchain* vkSwapchain = m_Swapchains.Find(swapchain);
		if (!vkSwapchain)
		{
			Debug().LogError("VulkanDevice::GetSwapchainImageCount: Invalid swap chain handle.");
			return 0;
		}
		return vkSwapchain->m_Textures[imageIndex];
	}

	void VulkanDevice::RecreateSwapchain(SwapchainHandle swapchain)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::RecreateSwapchain" };
		VK_Swapchain* vkSwapchain = m_Swapchains.Find(swapchain);
		if (!vkSwapchain)
		{
			Debug().LogError("VulkanDevice::RecreateSwapchain: Invalid swap chain handle.");
			return;
		}

		vk::SurfaceCapabilitiesKHR swapchainCapabilities;
		vk::SurfaceKHR surface;
		vkSwapchain->m_pWindow->GetVulkanSurface(GraphicsModule()->GetCInstance(), &surface);
		vk::Result vkError = m_VKDevice.getSurfaceCapabilitiesKHR(surface, &swapchainCapabilities);
		if (vkError != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::RecreateSwapchain: Failed to get surface capabilities: Vulkan Error Code: " + std::to_string((uint32_t)vkError));
			return;
		}

		std::vector<vk::SurfaceFormatKHR> swapchainFormats = m_VKDevice.getSurfaceFormatsKHR(surface);

		if (swapchainFormats.empty())
		{
			Debug().LogError("VulkanDevice::RecreateSwapchain: Surface does not support any formats");
			return;
		}

		int width, height;
		vkSwapchain->m_pWindow->GetDrawableSize(&width, &height);
		const glm::uvec2 resolution = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		if (resolution.x == 0 || resolution.y == 0)
			return;

		m_LogicalDevice.waitIdle();
		for (size_t i = 0; i < vkSwapchain->m_Textures.size(); ++i)
		{
			VK_Texture* vkTexture = m_Textures.Find(vkSwapchain->m_Textures[i]);
			if (!vkTexture) continue;
			m_LogicalDevice.destroyImageView(vkTexture->m_VKImageView);
		}
		m_LogicalDevice.destroySwapchainKHR(vkSwapchain->m_VKSwapchain);
		if (!CreateSwapchain(*vkSwapchain, swapchainCapabilities, surface, resolution, vkSwapchain->m_Vsync, vkSwapchain->m_MinImageCount))
		{
			Debug().LogError("VulkanDevice::RecreateSwapchain: Failed to create swapchain!");
			return;
		}

		std::stringstream str;
		str << "VulkanDevice: Swap chain " << swapchain << " was recreated.";
		Debug().LogInfo(str.str());
	}

	SemaphoreHandle VulkanDevice::CreateSemaphore()
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateSemaphore" };
		SemaphoreHandle handle;
		VK_Semaphore& semaphore = m_Semaphores.Emplace(handle, VK_Semaphore());
		vk::SemaphoreCreateInfo createInfo;
		if (m_LogicalDevice.createSemaphore(&createInfo, nullptr, &semaphore.m_VKSemaphore) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateSemaphore: Failed to create semaphore");
			return NULL;
		}
		return handle;
	}

	void VulkanDevice::FreeBuffer(BufferHandle& handle)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::FreeBuffer" };
		VK_Buffer* buffer = m_Buffers.Find(handle);
		if (!buffer)
		{
			Debug().LogError("VulkanDevice::FreeBuffer: Invalid buffer handle.");
			return;
		}

		if (buffer->m_pMappedMemory)
		{
			m_LogicalDevice.unmapMemory(buffer->m_VKMemory);
			buffer->m_pMappedMemory = nullptr;
		}

		m_LogicalDevice.destroyBuffer(buffer->m_VKBuffer);
		m_LogicalDevice.freeMemory(buffer->m_VKMemory);
		m_Buffers.Erase(handle);

		std::stringstream str;
		str << "VulkanDevice: Buffer " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}

	void VulkanDevice::FreeMesh(MeshHandle& handle)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::FreeMesh" };
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
		ProfileSample s{ &Profiler(), "VulkanDevice::FreeTexture" };
		VK_Texture* texture = m_Textures.Find(handle);
		if (!texture)
		{
			Debug().LogError("VulkanDevice::FreeTexture: Invalid texture handle.");
			return;
		}

		if (texture->m_IsSwapchainImage) return;
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
		ProfileSample s{ &Profiler(), "VulkanDevice::FreeRenderTexture" };
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
		ProfileSample s{ &Profiler(), "VulkanDevice::FreeRenderPass" };
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
		ProfileSample s{ &Profiler(), "VulkanDevice::FreeShader" };
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
		ProfileSample s{ &Profiler(), "VulkanDevice::FreePipeline" };
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

	void VulkanDevice::FreeDescriptorSetLayout(DescriptorSetLayoutHandle& handle)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::FreeDescriptorSetLayout" };
		VK_DescriptorSetLayout* vkSetLayout = m_DescriptorSetLayouts.Find(handle);
		if (!vkSetLayout)
		{
			Debug().LogError("VulkanDevice::FreeDescriptorSetLayout: Invalid descriptor set layout handle.");
			return;
		}

		m_LogicalDevice.destroyDescriptorSetLayout(vkSetLayout->m_VKLayout);
		m_DescriptorSetLayouts.Erase(handle);

		std::stringstream str;
		str << "VulkanDevice: Descriptor set layout " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}

	void VulkanDevice::FreeDescriptorSet(DescriptorSetHandle& handle)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::FreeDescriptorSet" };
		VK_DescriptorSet* vkSet = m_DescriptorSets.Find(handle);
		if (!vkSet)
		{
			Debug().LogError("VulkanDevice::FreeDescriptorSetLayout: Invalid descriptor set handle.");
			return;
		}
		
		m_LogicalDevice.freeDescriptorSets(vkSet->m_VKDescriptorPool, 1, &vkSet->m_VKDescriptorSet);
		m_DescriptorSets.Erase(handle);

		std::stringstream str;
		str << "VulkanDevice: Descriptor set " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}

	void VulkanDevice::FreeSwapchain(SwapchainHandle& handle)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::FreeSwapchain" };
		VK_Swapchain* vkSwapchain = m_Swapchains.Find(handle);
		if (!vkSwapchain)
		{
			Debug().LogError("VulkanDevice::FreeSwapchain: Invalid swap chain handle.");
			return;
		}

		m_LogicalDevice.destroySwapchainKHR(vkSwapchain->m_VKSwapchain);

		for (TextureHandle texture : vkSwapchain->m_Textures)
		{
			VK_Texture* vkTexture = m_Textures.Find(texture);
			if (!vkTexture) continue;
			m_LogicalDevice.destroyImageView(vkTexture->m_VKImageView);
		}
		vkSwapchain->m_Textures.clear();

		m_Swapchains.Erase(handle);

		std::stringstream str;
		str << "VulkanDevice: Swap chain " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}

	void VulkanDevice::FreeSemaphore(SemaphoreHandle& handle)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::FreeSemaphore" };
		VK_Semaphore* vkSemaphore = m_Semaphores.Find(handle);
		if (!vkSemaphore)
		{
			Debug().LogError("VulkanDevice::FreeSemaphore: Invalid semaphore handle.");
			return;
		}

		m_LogicalDevice.destroySemaphore(vkSemaphore->m_VKSemaphore);

		m_Semaphores.Erase(handle);

		std::stringstream str;
		str << "VulkanDevice: Semaphore " << handle << " was freed from device memory.";
		Debug().LogInfo(str.str());

		handle = 0;
	}

	vk::CommandBuffer VulkanDevice::BeginSingleTimeCommands()
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::BeginSingleTimeCommands" };
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
		ProfileSample s{ &Profiler(), "VulkanDevice::EndSingleTimeCommands" };
		vk::CommandPool commandPool = GetGraphicsCommandPool();

		commandBuffer.end();

		vk::SubmitInfo submitInfo = vk::SubmitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		GraphicsQueue().submit(1, &submitInfo, VK_NULL_HANDLE);
		GraphicsQueue().waitIdle();
		m_LogicalDevice.freeCommandBuffers(commandPool, 1, &commandBuffer);
	}

	void VulkanDevice::TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::TransitionImageLayout" };
		vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

		vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier();
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspectFlags;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
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
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = (vk::AccessFlags)0;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal)
		{
			barrier.srcAccessMask = (vk::AccessFlags)0;
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal ||
			newLayout == vk::ImageLayout::eDepthAttachmentOptimal || newLayout == vk::ImageLayout::eStencilAttachmentOptimal))
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

	void VulkanDevice::GenerateMipMaps(vk::Image image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::GenerateMipMaps" };
		vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

		vk::ImageMemoryBarrier barrier{};
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; ++i)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eTransfer, (vk::DependencyFlags)0,
				0, nullptr, 0, nullptr, 1, &barrier);

			vk::ImageBlit blit{};
			blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
			blit.srcOffsets[1] = vk::Offset3D{ mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
			blit.dstOffsets[1] = vk::Offset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal,
				image, vk::ImageLayout::eTransferDstOptimal, 1, &blit, vk::Filter::eLinear);

			barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
				(vk::DependencyFlags)0, 0, nullptr, 0, nullptr, 1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
			(vk::DependencyFlags)0, 0, nullptr, 0, nullptr, 1, &barrier);

		EndSingleTimeCommands(commandBuffer);
	}

	void VulkanDevice::CopyFromBuffer(vk::Buffer buffer, vk::Image image, vk::ImageAspectFlags aspectFlags, uint32_t width, uint32_t height)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CopyFromBuffer" };
		CopyFromBuffer(buffer, image, aspectFlags, 0, 0, 0, width, height, 1);
	}

	void VulkanDevice::CopyFromBuffer(vk::Buffer buffer, vk::Image image, vk::ImageAspectFlags aspectFlags, int32_t offsetX, int32_t offsetY, int32_t offsetZ, uint32_t width, uint32_t height, uint32_t depth)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CopyFromBuffer(offset)" };
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

	void VulkanDevice::CopyFromBuffer(vk::Buffer dst, vk::Buffer src, int32_t dstOffset, int32_t srcOffset, uint32_t size)
	{
		vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();
		vk::BufferCopy copyRegion = vk::BufferCopy();
		copyRegion.dstOffset = dstOffset;
		copyRegion.srcOffset = srcOffset;
		copyRegion.size = size;
		commandBuffer.copyBuffer(src, dst, 1, &copyRegion);
		EndSingleTimeCommands(commandBuffer);
	}

	vk::CommandBuffer VulkanDevice::GetNewResetableCommandBuffer(CommandBufferHandle commandBufferHandle)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::GetNewResetableCommandBuffer" };
		if (m_FreeCommandBuffers.empty())
			AllocateFreeCommandBuffers(10);
		if (m_FreeFences.empty())
			AllocateFreeFences(10);

		vk::CommandBuffer commandBuffer = m_FreeCommandBuffers.back();
		VK_CommandBuffer& vkCommandBuffer = m_CommandBuffers.emplace(commandBufferHandle, VK_CommandBuffer()).first->second;
		vkCommandBuffer.m_VKCommandBuffer = m_FreeCommandBuffers.back();
		vkCommandBuffer.m_VKFence = m_FreeFences.back();
		vkCommandBuffer.m_Resetable = true;
		m_FreeCommandBuffers.erase(--m_FreeCommandBuffers.end());
		m_FreeFences.erase(--m_FreeFences.end());
		return commandBuffer;
	}

	vk::PresentModeKHR VulkanDevice::SelectPresentMode(const std::vector<vk::PresentModeKHR>& presentModes, vk::SurfaceKHR surface)
	{
		std::vector<vk::PresentModeKHR> availablePresentModes;
		uint32_t presentModesCount = 0;
		m_VKDevice.getSurfacePresentModesKHR(surface, &presentModesCount, nullptr);
		availablePresentModes.resize(presentModesCount);
		m_VKDevice.getSurfacePresentModesKHR(surface, &presentModesCount, availablePresentModes.data());

		for (vk::PresentModeKHR chosenPresentMode : presentModes)
		{
			if (std::find(availablePresentModes.begin(), availablePresentModes.end(), chosenPresentMode) == availablePresentModes.end())
				continue;
			return chosenPresentMode;
		}
		return vk::PresentModeKHR::eFifo;
	}

	vk::SurfaceFormatKHR VulkanDevice::SelectSurfaceFormat(vk::SurfaceKHR surface, const std::vector<vk::Format> requestFormats, vk::ColorSpaceKHR requestColorSpace)
	{
		uint32_t availableFormatCount;
		std::vector<vk::SurfaceFormatKHR> availableFormats;
		m_VKDevice.getSurfaceFormatsKHR(surface, &availableFormatCount, nullptr);
		availableFormats.resize(availableFormatCount);
		m_VKDevice.getSurfaceFormatsKHR(surface, &availableFormatCount, availableFormats.data());

		// If only vk::Format::eUndefined is available then every format is allowed
		if (availableFormatCount == 1)
		{
			if (availableFormats[0].format == vk::Format::eUndefined)
			{
				vk::SurfaceFormatKHR ret;
				ret.format = requestFormats[0];
				ret.colorSpace = requestColorSpace;
				return ret;
			}

			// No point in searching another format
			return availableFormats[0];
		}

		// Request several formats, the first found will be used
		for (vk::Format chosenFormat : requestFormats)
		{
			auto iter = std::find_if(availableFormats.begin(), availableFormats.end(), [requestColorSpace, chosenFormat](vk::SurfaceFormatKHR& format) {
				return format.colorSpace == requestColorSpace && format.format == chosenFormat;
			});

			if (iter == availableFormats.end())
				continue;
			return *iter;
		}

		// If none of the requested image formats could be found, use the first available
		return availableFormats[0];
	}

	void VulkanDevice::CreateRenderTexture(vk::RenderPass vkRenderPass, VK_RenderTexture& renderTexture)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateRenderTexture" };
		const size_t numAttachments = renderTexture.m_Info.Attachments.size() + (renderTexture.m_HasDepthOrStencil ? 1 : 0);
		renderTexture.m_AttachmentNames.resize(numAttachments);
		renderTexture.m_Textures.resize(numAttachments);

		SamplerSettings sampler;
		sampler.MipmapMode = Filter::F_None;
		sampler.MinFilter = Filter::F_None;
		sampler.MagFilter = Filter::F_None;

		size_t textureCounter = 0;
		for (size_t i = 0; i < renderTexture.m_Info.Attachments.size(); ++i)
		{
			const Attachment& attachment = renderTexture.m_Info.Attachments[i];
			renderTexture.m_Textures[i] = attachment.Texture ? attachment.Texture :
				CreateTexture({ renderTexture.m_Info.Width, renderTexture.m_Info.Height, attachment.Format, attachment.InternalFormat, attachment.ImageType, attachment.m_Type, 0, 0, attachment.ImageAspect, sampler, attachment.m_SamplingEnabled });
			VK_Texture* vkTexture = m_Textures.Find(renderTexture.m_Textures[i]);
			vkTexture->m_VKFinalLayout = attachment.m_SamplingEnabled ? vk::ImageLayout::eShaderReadOnlyOptimal : vk::ImageLayout::eColorAttachmentOptimal;
			renderTexture.m_AttachmentNames[i] = attachment.Name;
			++textureCounter;
		}

		size_t depthStencilIndex = 0;

		if (renderTexture.m_Info.HasDepth && renderTexture.m_Info.HasStencil)
		{
			depthStencilIndex = textureCounter;
			renderTexture.m_Textures[depthStencilIndex] = CreateTexture({ renderTexture.m_Info.Width, renderTexture.m_Info.Height, PixelFormat::PF_Depth, PixelFormat::PF_D32SfloatS8Uint, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Depth, sampler });
			VK_Texture* vkTexture = m_Textures.Find(renderTexture.m_Textures[depthStencilIndex]);
			vkTexture->m_VKFinalLayout = renderTexture.m_Info.EnableDepthStencilSampling ? vk::ImageLayout::eShaderReadOnlyOptimal : vk::ImageLayout::eStencilAttachmentOptimal;
			renderTexture.m_AttachmentNames[depthStencilIndex] = "DepthStencil";
			++textureCounter;
		}
		else if (renderTexture.m_Info.HasDepth)
		{
			depthStencilIndex = textureCounter;
			renderTexture.m_Textures[depthStencilIndex] = CreateTexture({ renderTexture.m_Info.Width, renderTexture.m_Info.Height, PixelFormat::PF_Depth, PixelFormat::PF_D32Sfloat, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Depth, sampler });
			VK_Texture* vkTexture = m_Textures.Find(renderTexture.m_Textures[depthStencilIndex]);
			vkTexture->m_VKFinalLayout = renderTexture.m_Info.EnableDepthStencilSampling ? vk::ImageLayout::eShaderReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal;
			renderTexture.m_AttachmentNames[depthStencilIndex] = "Depth";
			++textureCounter;
		}
		else if (renderTexture.m_Info.HasStencil)
		{
			depthStencilIndex = textureCounter;
			renderTexture.m_Textures[depthStencilIndex] = CreateTexture({ renderTexture.m_Info.Width, renderTexture.m_Info.Height, PixelFormat::PF_Stencil, PixelFormat::PF_R8Uint, ImageType::IT_2D, DataType::DT_UInt, 0, 0, ImageAspect::IA_Stencil, sampler });
			VK_Texture* vkTexture = m_Textures.Find(renderTexture.m_Textures[depthStencilIndex]);
			vkTexture->m_VKFinalLayout = renderTexture.m_Info.EnableDepthStencilSampling ? vk::ImageLayout::eShaderReadOnlyOptimal : vk::ImageLayout::eDepthStencilAttachmentOptimal;
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

		if (renderTexture.m_Info.HasDepth || renderTexture.m_Info.HasStencil)
		{
			VK_Texture* texture = m_Textures.Find(renderTexture.m_Textures[depthStencilIndex]);
			attachments[depthStencilIndex] = texture->m_VKImageView;
		}

		vk::FramebufferCreateInfo frameBufferCreateInfo = vk::FramebufferCreateInfo()
			.setRenderPass(vkRenderPass)
			.setAttachmentCount(attachments.size())
			.setPAttachments(attachments.data())
			.setWidth(renderTexture.m_Info.Width)
			.setHeight(renderTexture.m_Info.Height)
			.setLayers(1);

		renderTexture.m_VKFramebuffer = m_LogicalDevice.createFramebuffer(frameBufferCreateInfo);
		if (renderTexture.m_VKFramebuffer == nullptr)
			Debug().LogError("VulkanDevice::CreateRenderTexture: There was an error when trying to create a frame buffer.");
	}

	bool VulkanDevice::CreateSwapchain(VK_Swapchain& swapchain, const vk::SurfaceCapabilitiesKHR& capabilities,
		vk::SurfaceKHR surface, const glm::uvec2& resolution, bool vsync, uint32_t minImageCount)
	{
		ProfileSample s{ &Profiler(), "VulkanDevice::CreateSwapchain" };
		/* Get shaw chain format */
		const vk::SurfaceFormatKHR swapchainFormat = SelectSurfaceFormat(surface,
			{ vk::Format::eR8G8B8A8Unorm, vk::Format::eB8G8R8A8Unorm, vk::Format::eR8G8B8Unorm, vk::Format::eB8G8R8Unorm },
			vk::ColorSpaceKHR::eSrgbNonlinear);

		/* Get shaw chain present mode */
		const vk::PresentModeKHR swapchainPresentMode = SelectPresentMode(vsync ? VSyncPresentModes : InfiniteFPSPresentModes, surface);
		if (minImageCount == 0)
			minImageCount = GetMinImageCountFromPresentMode(swapchainPresentMode);

		/* Get swap chain extend */
		vk::Extent2D swapchainExtent = capabilities.currentExtent;
		if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
		{
			VkExtent2D actualExtent = {
				resolution.x,
				resolution.y
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			swapchainExtent = actualExtent;
		}

		if (minImageCount < capabilities.minImageCount) {
			minImageCount = capabilities.minImageCount;
		}
		if (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount) {
			minImageCount = capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo{};
		createInfo.surface = surface;
		createInfo.minImageCount = minImageCount;
		createInfo.imageFormat = swapchainFormat.format;
		createInfo.imageColorSpace = swapchainFormat.colorSpace;
		createInfo.imageExtent = swapchainExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		uint32_t queueFamilyIndices[] = { m_GraphicsAndComputeFamily.value(), m_PresentFamily.value() };

		if (m_GraphicsAndComputeFamily.value() != m_PresentFamily.value())
		{
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = swapchainPresentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (m_LogicalDevice.createSwapchainKHR(&createInfo, nullptr, &swapchain.m_VKSwapchain) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateSwapchain: Failed to create swapchain");
			return false;
		}

		uint32_t imageCount;
		if (m_LogicalDevice.getSwapchainImagesKHR(swapchain.m_VKSwapchain, &imageCount, nullptr) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateSwapchain: Failed to get swap chain images");
			return false;
		}
		std::vector<vk::Image> swapchainImages(imageCount);
		std::vector<vk::ImageView> swapchainImageViews(imageCount);
		if (m_LogicalDevice.getSwapchainImagesKHR(swapchain.m_VKSwapchain, &imageCount, swapchainImages.data()) != vk::Result::eSuccess)
		{
			Debug().LogError("VulkanDevice::CreateSwapchain: Failed to get swap chain images");
			return false;
		}

		swapchain.m_Format = swapchainFormat;
		swapchain.m_PresentMode = swapchainPresentMode;
		swapchain.m_Extent = swapchainExtent;
		swapchain.m_MinImageCount = imageCount;

		for (size_t i = 0; i < imageCount; ++i)
		{
			vk::ImageViewCreateInfo imageViewInfo;
			imageViewInfo.image = swapchainImages[i];
			imageViewInfo.viewType = vk::ImageViewType::e2D;
			imageViewInfo.format = swapchainFormat.format;
			imageViewInfo.components.r = vk::ComponentSwizzle::eIdentity;
			imageViewInfo.components.g = vk::ComponentSwizzle::eIdentity;
			imageViewInfo.components.b = vk::ComponentSwizzle::eIdentity;
			imageViewInfo.components.a = vk::ComponentSwizzle::eIdentity;
			imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			imageViewInfo.subresourceRange.baseMipLevel = 0;
			imageViewInfo.subresourceRange.levelCount = 1;
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.layerCount = 1;
			if (m_LogicalDevice.createImageView(&imageViewInfo, nullptr, &swapchainImageViews[i]) != vk::Result::eSuccess)
			{
				Debug().LogError("VulkanDevice::CreateSwapchain: Failed to create image view");
				return false;
			}
		}

		swapchain.m_Textures.resize(imageCount);
		for (size_t i = 0; i < imageCount; ++i)
		{
			VK_Texture* vkTexture = m_Textures.Find(swapchain.m_Textures[i]);
			if (!vkTexture)
			{
				swapchain.m_Textures[i] = TextureHandle();
				vkTexture = &m_Textures.Emplace(swapchain.m_Textures[i], VK_Texture());
			}
			vkTexture->m_VKInitialLayout = vk::ImageLayout::eUndefined;
			vkTexture->m_VKFinalLayout = vk::ImageLayout::ePresentSrcKHR;
			vkTexture->m_VKImage = swapchainImages[i];
			vkTexture->m_VKImageView = swapchainImageViews[i];
			vkTexture->m_VKMemory = nullptr;
			vkTexture->m_VKSampler = nullptr;
			vkTexture->m_VKFormat = swapchainFormat.format;
			vkTexture->m_IsSwapchainImage = true;
		}

		return true;
	}
}
