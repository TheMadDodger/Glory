#include "EditorVulkanRenderImpl.h"

#include <EditorApplication.h>
#include <EditorShaderData.h>

#include <VulkanDevice.h>

namespace Glory::Editor
{
	void LoadBackend(EditorCreateInfo& editorCreateInfo)
	{
		editorCreateInfo.pRenderImpl = new EditorVulkanRenderImpl();
	}

	static void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

	EditorVulkanRenderImpl::EditorVulkanRenderImpl() {}

	EditorVulkanRenderImpl::~EditorVulkanRenderImpl() {}

	void* EditorVulkanRenderImpl::GetTextureID(Texture* pTexture)
	{
		VulkanTexture* pVKTexture = static_cast<VulkanTexture*>(pTexture);
		auto iter = m_DesciptorSets.find(pTexture->ID());
		if (iter == m_DesciptorSets.end())
		{
			vk::DescriptorSet ds = ImGui_ImplVulkan_AddTexture(pVKTexture->GetTextureSampler(), pVKTexture->GetTextureImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			iter = m_DesciptorSets.emplace(pTexture->ID(), ds).first;
		}
		return (void*)iter->second;
	}

	void* EditorVulkanRenderImpl::GetTextureID(TextureHandle texture)
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		m_pDevice = static_cast<VulkanDevice*>(pEngine->ActiveGraphicsDevice());
		auto iter = m_DesciptorSets.find(texture);
		if (iter == m_DesciptorSets.end())
		{
			vk::DescriptorSet ds = ImGui_ImplVulkan_AddTexture(m_pDevice->GetVKSampler(texture), m_pDevice->GetVKImageView(texture), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			iter = m_DesciptorSets.emplace(texture, ds).first;
		}
		return (void*)iter->second;
	}

	std::string EditorVulkanRenderImpl::ShadingLanguage()
	{
		return "spv";
	}

	void EditorVulkanRenderImpl::CompileShaderForEditor(const EditorShaderData& editorShader, std::vector<char>& out)
	{
		out.resize(editorShader.Size()*sizeof(uint32_t));
		std::memcpy(out.data(), editorShader.Data(), editorShader.Size()*sizeof(uint32_t));
	}

	void EditorVulkanRenderImpl::SetContext(ImGuiContext* pImguiConext)
	{
		ImGui::SetCurrentContext(pImguiConext);
	}

	void EditorVulkanRenderImpl::Setup()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		VulkanGraphicsModule* pGraphicsModule = pEngine->GetMainModule<VulkanGraphicsModule>();
		VkInstance instance = pGraphicsModule->GetCInstance();
		m_pDevice = static_cast<VulkanDevice*>(pEngine->ActiveGraphicsDevice());

		m_MainWindow = ImGui_ImplVulkanH_Window();
		//m_MainWindow.Swapchain = pGraphicsModule->GetSwapChain().GetSwapChain();
		//m_MainWindow.RenderPass = pGraphicsModule->MainRenderPass().GetRenderPass();
		//m_MainWindow.ImageCount = pGraphicsModule->ImageCount();
		VkSurfaceKHR surface = pGraphicsModule->GetCSurface();
		int width, height;

		m_pEditorPlatform->GetWindowImpl()->GetMainWindow()->GetWindowSize(&width, &height);
		SetupVulkanWindow(instance, &m_MainWindow, surface, width, height);
	}

	void EditorVulkanRenderImpl::SetupBackend()
	{
		m_pEditorPlatform->GetWindowImpl()->SetupForVulkan();

		VulkanGraphicsModule* pGraphicsModule = EditorApplication::GetInstance()->GetEngine()->GetMainModule<VulkanGraphicsModule>();

		// Instance
		ImGui_ImplVulkan_InitInfo vulkanInfo{};
		vulkanInfo.Instance = pGraphicsModule->GetCInstance();

		// Device and queues
		vulkanInfo.PhysicalDevice = (VkPhysicalDevice)m_pDevice->PhysicalDevice();
		vulkanInfo.Device = (VkDevice)m_pDevice->LogicalDevice();
		vulkanInfo.QueueFamily = m_pDevice->GraphicsFamily();
		vulkanInfo.Queue = m_pDevice->GraphicsQueue();

		// Not used
		vulkanInfo.PipelineCache = VK_NULL_HANDLE;
		vulkanInfo.Allocator = VK_NULL_HANDLE;

		// Descriptor pool
		CreateDescriptorPool(vulkanInfo.Device);
		vulkanInfo.DescriptorPool = m_DescriptorPool;

		// Debug
		vulkanInfo.CheckVkResultFn = check_vk_result;

		// Image count
		vulkanInfo.MinImageCount = MINIMAGECOUNT;
		vulkanInfo.ImageCount = m_MainWindow.ImageCount;

		ImGui_ImplVulkan_Init(&vulkanInfo, m_MainWindow.RenderPass);
	}

	void EditorVulkanRenderImpl::UploadImGUIFonts()
	{
		UploadFonts(&m_MainWindow);
	}

	void EditorVulkanRenderImpl::Shutdown()
	{
		VkResult err = vkDeviceWaitIdle((VkDevice)m_pDevice->LogicalDevice());
		check_vk_result(err);
		ImGui_ImplVulkan_Shutdown();
	}

	void EditorVulkanRenderImpl::Cleanup()
	{
		CleanupVulkanWindow();
		CleanupVulkan();
	}

	void EditorVulkanRenderImpl::SetupVulkanWindow(VkInstance instance, ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
	{
		wd->Surface = surface;

		VkPhysicalDevice physicalDevice = (VkPhysicalDevice)m_pDevice->PhysicalDevice();
		VkDevice device = (VkDevice)m_pDevice->LogicalDevice();
		uint32_t graphicsFamilyIndex = m_pDevice->GraphicsFamily();

		// Check for WSI support
		VkBool32 res;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, graphicsFamilyIndex, wd->Surface, &res);
		if (res != VK_TRUE)
		{
			fprintf(stderr, "Error no WSI support on physical device 0\n");
			exit(-1);
		}

		// Select Surface Format
		const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
		const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(physicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

		// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
		wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(physicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
		//printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

		// Create SwapChain, RenderPass, Framebuffer, etc.
		IM_ASSERT(MINIMAGECOUNT >= 2);
		ImGui_ImplVulkanH_CreateOrResizeWindow(instance, physicalDevice, device, wd, graphicsFamilyIndex, VK_NULL_HANDLE, width, height, MINIMAGECOUNT);
	}

	void EditorVulkanRenderImpl::CreateDescriptorPool(VkDevice device)
	{
		VkResult err;

		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		err = vkCreateDescriptorPool(device, &pool_info, NULL, &m_DescriptorPool);
		check_vk_result(err);
	}

	void EditorVulkanRenderImpl::UploadFonts(ImGui_ImplVulkanH_Window* wd)
	{
		VkPhysicalDevice physicalDevice = (VkPhysicalDevice)m_pDevice->PhysicalDevice();
		VkDevice device = (VkDevice)m_pDevice->LogicalDevice();
		uint32_t graphicsFamilyIndex = m_pDevice->GraphicsFamily();

		VkResult err;
		// Use any command queue
		VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
		VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

		err = vkResetCommandPool(device, command_pool, 0);
		check_vk_result(err);
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(command_buffer, &begin_info);
		check_vk_result(err);

		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

		VkSubmitInfo end_info = {};
		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &command_buffer;
		err = vkEndCommandBuffer(command_buffer);
		check_vk_result(err);
		err = vkQueueSubmit(m_pDevice->GraphicsQueue(), 1, &end_info, VK_NULL_HANDLE);
		check_vk_result(err);

		err = vkDeviceWaitIdle(device);
		check_vk_result(err);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void EditorVulkanRenderImpl::FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
	{
		VkPhysicalDevice physicalDevice = (VkPhysicalDevice)m_pDevice->PhysicalDevice();
		VkDevice device = (VkDevice)m_pDevice->LogicalDevice();
		uint32_t graphicsFamilyIndex = m_pDevice->GraphicsFamily();

		VkResult err;

		VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
		VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
		err = vkAcquireNextImageKHR(device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
		if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChainRebuild = true;
			return;
		}
		check_vk_result(err);

		ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
		{
			err = vkWaitForFences(device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
			check_vk_result(err);

			err = vkResetFences(device, 1, &fd->Fence);
			check_vk_result(err);
		}
		{
			err = vkResetCommandPool(device, fd->CommandPool, 0);
			check_vk_result(err);
			VkCommandBufferBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
			check_vk_result(err);
		}
		{
			VkRenderPassBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = wd->RenderPass;
			info.framebuffer = fd->Framebuffer;
			info.renderArea.extent.width = wd->Width;
			info.renderArea.extent.height = wd->Height;
			info.clearValueCount = 1;
			info.pClearValues = &wd->ClearValue;
			vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		// Record dear imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

		// Submit command buffer
		vkCmdEndRenderPass(fd->CommandBuffer);
		{
			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkSubmitInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.waitSemaphoreCount = 1;
			info.pWaitSemaphores = &image_acquired_semaphore;
			info.pWaitDstStageMask = &wait_stage;
			info.commandBufferCount = 1;
			info.pCommandBuffers = &fd->CommandBuffer;
			info.signalSemaphoreCount = 1;
			info.pSignalSemaphores = &render_complete_semaphore;

			err = vkEndCommandBuffer(fd->CommandBuffer);
			check_vk_result(err);
			err = vkQueueSubmit(m_pDevice->GraphicsQueue(), 1, &info, fd->Fence);
			check_vk_result(err);
		}
	}

	void EditorVulkanRenderImpl::FramePresent(ImGui_ImplVulkanH_Window* wd)
	{
		VkPhysicalDevice physicalDevice = (VkPhysicalDevice)m_pDevice->PhysicalDevice();
		VkDevice device = (VkDevice)m_pDevice->LogicalDevice();
		uint32_t graphicsFamilyIndex = m_pDevice->GraphicsFamily();

		if (m_SwapChainRebuild)
			return;

		VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &render_complete_semaphore;
		info.swapchainCount = 1;
		info.pSwapchains = &wd->Swapchain;
		info.pImageIndices = &wd->FrameIndex;
		VkResult err = vkQueuePresentKHR((VkQueue)m_pDevice->PresentQueue(), &info);
		if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
		{
			m_SwapChainRebuild = true;
			return;
		}
		check_vk_result(err);
		wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
	}

	void EditorVulkanRenderImpl::CleanupVulkanWindow()
	{
		VkPhysicalDevice physicalDevice = (VkPhysicalDevice)m_pDevice->PhysicalDevice();
		VkDevice device = (VkDevice)m_pDevice->LogicalDevice();
		uint32_t graphicsFamilyIndex = m_pDevice->GraphicsFamily();

		VulkanGraphicsModule* pGraphicsModule = EditorApplication::GetInstance()->GetEngine()->GetMainModule<VulkanGraphicsModule>();

		ImGui_ImplVulkanH_DestroyWindow(pGraphicsModule->GetCInstance(), device, &m_MainWindow, VK_NULL_HANDLE);
	}

	void EditorVulkanRenderImpl::CleanupVulkan()
	{
		VkPhysicalDevice physicalDevice = (VkPhysicalDevice)m_pDevice->PhysicalDevice();
		VkDevice device = (VkDevice)m_pDevice->LogicalDevice();

		vkDestroyDescriptorPool(device, m_DescriptorPool, VK_NULL_HANDLE);
	}

	void EditorVulkanRenderImpl::BeforeRender()
	{
		VkPhysicalDevice physicalDevice = (VkPhysicalDevice)m_pDevice->PhysicalDevice();
		VkDevice device = (VkDevice)m_pDevice->LogicalDevice();
		uint32_t graphicsFamilyIndex = m_pDevice->GraphicsFamily();

		// Resize swap chain?
		if (m_SwapChainRebuild)
		{
			int width, height;
			m_pEditorPlatform->GetWindowImpl()->GetMainWindow()->GetWindowSize(&width, &height);
			if (width > 0 && height > 0)
			{
				ImGui_ImplVulkan_SetMinImageCount(MINIMAGECOUNT);
				ImGui_ImplVulkanH_CreateOrResizeWindow(m_pDevice->GraphicsModule()->GetCInstance(), physicalDevice, device, &m_MainWindow, graphicsFamilyIndex, VK_NULL_HANDLE, width, height, MINIMAGECOUNT);
				m_MainWindow.FrameIndex = 0;
				m_SwapChainRebuild = false;
			}
		}
	}

	void EditorVulkanRenderImpl::NewFrame()
	{
		ImGui_ImplVulkan_NewFrame();
	}

	void EditorVulkanRenderImpl::Clear(const ImVec4& clearColor)
	{
		m_MainWindow.ClearValue.color.float32[0] = clearColor.x * clearColor.w;
		m_MainWindow.ClearValue.color.float32[1] = clearColor.y * clearColor.w;
		m_MainWindow.ClearValue.color.float32[2] = clearColor.z * clearColor.w;
		m_MainWindow.ClearValue.color.float32[3] = clearColor.w;
	}

	void EditorVulkanRenderImpl::FrameRender(ImDrawData* pDrawData)
	{
		FrameRender(&m_MainWindow, pDrawData);
	}

	void EditorVulkanRenderImpl::FramePresent()
	{
		FramePresent(&m_MainWindow);
	}
}
