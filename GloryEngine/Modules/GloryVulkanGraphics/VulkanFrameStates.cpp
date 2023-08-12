#include "VulkanFrameStates.h"
#include "Device.h"

namespace Glory
{
	size_t VulkanFrameStates::m_CurrentFrame = 0;
	size_t VulkanFrameStates::m_CurrentImageIndex = 0;
	const size_t VulkanFrameStates::MAX_FRAMES_IN_FLIGHT = 2;

	VulkanFrameStates::VulkanFrameStates(VulkanGraphicsModule* pModule) : m_pModule(pModule), FrameStates(pModule)
	{
	}

	VulkanFrameStates::~VulkanFrameStates()
	{
	}

	size_t VulkanFrameStates::GetCurrentImageIndex()
	{
		return m_CurrentImageIndex;
	}

	void VulkanFrameStates::Initialize()
	{
		LogicalDeviceData deviceData = m_pModule->GetDeviceManager()->GetSelectedDevice()->GetLogicalDeviceData();

		// Create sync objects
		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_ImagesInFlight.resize(m_pModule->GetSwapChain()->GetImageCount(), VK_NULL_HANDLE);

		vk::SemaphoreCreateInfo semaphoreCreateInfo = vk::SemaphoreCreateInfo();
		vk::FenceCreateInfo fenceCreateInfo = vk::FenceCreateInfo()
			.setFlags(vk::FenceCreateFlagBits::eSignaled);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (deviceData.LogicalDevice.createSemaphore(&semaphoreCreateInfo, nullptr, &m_ImageAvailableSemaphores[i]) != vk::Result::eSuccess ||
				deviceData.LogicalDevice.createSemaphore(&semaphoreCreateInfo, nullptr, &m_RenderFinishedSemaphores[i]) != vk::Result::eSuccess ||
				deviceData.LogicalDevice.createFence(&fenceCreateInfo, nullptr, &m_InFlightFences[i]) != vk::Result::eSuccess)
			{

				throw std::runtime_error("failed to create sync objects for a frame!");
			}
		}
	}

	void VulkanFrameStates::OnFrameStart()
	{
		LogicalDeviceData deviceData = m_pModule->GetDeviceManager()->GetSelectedDevice()->GetLogicalDeviceData();

		deviceData.LogicalDevice.waitForFences(1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
		deviceData.LogicalDevice.acquireNextImageKHR(m_pModule->GetSwapChain()->GetSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_CurrentImageIndex);

		// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (m_ImagesInFlight[m_CurrentImageIndex] != VK_NULL_HANDLE) {
			deviceData.LogicalDevice.waitForFences(1, &m_ImagesInFlight[m_CurrentImageIndex], VK_TRUE, UINT64_MAX);
		}
		// Mark the image as now being in use by this frame
		m_ImagesInFlight[m_CurrentImageIndex] = m_InFlightFences[m_CurrentFrame];

		//UpdateUniformBuffer(imageIndex);

		// Begin the current frame command buffer
		vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
			.setPInheritanceInfo(nullptr);

		vk::CommandBuffer commandBuffer = m_pModule->GetVulkanCommandBuffers()->GetCurrentFrameCommandBuffer();
		if(commandBuffer.begin(&commandBufferBeginInfo) != vk::Result::eSuccess)
			throw std::runtime_error("failed to begin recording command buffer!");
	}

	void VulkanFrameStates::OnFrameEnd()
	{
		LogicalDeviceData deviceData = m_pModule->GetDeviceManager()->GetSelectedDevice()->GetLogicalDeviceData();
		VulkanCommandBuffers* pCommandBuffers = m_pModule->GetVulkanCommandBuffers();

		// End the current frame command buffer
		vk::CommandBuffer commandBuffer = pCommandBuffers->GetCurrentFrameCommandBuffer();
		commandBuffer.end();
		
		// Submit command buffer
		vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		vk::SubmitInfo submitInfo = vk::SubmitInfo()
		    .setWaitSemaphoreCount(1)
		    .setPWaitSemaphores(waitSemaphores)
		    .setPWaitDstStageMask(waitStages)
		    .setCommandBufferCount(1)
		    .setPCommandBuffers(&commandBuffer)
		    .setSignalSemaphoreCount(1)
		    .setPSignalSemaphores(signalSemaphores);
		
		deviceData.LogicalDevice.resetFences(1, &m_InFlightFences[m_CurrentFrame]);
		
		if (deviceData.GraphicsQueue.submit(1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != vk::Result::eSuccess)
		    throw std::runtime_error("failed to submit draw command buffer!");
		
		vk::SwapchainKHR swapChains[] = { m_pModule->GetSwapChain()->GetSwapChain() };
		vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
		    .setWaitSemaphoreCount(1)
		    .setPWaitSemaphores(signalSemaphores)
		    .setSwapchainCount(1)
		    .setPSwapchains(swapChains)
		    .setPImageIndices(&m_CurrentImageIndex)
		    .setPResults(nullptr);
		
		if (deviceData.PresentQueue.presentKHR(&presentInfo) != vk::Result::eSuccess)
		    throw std::runtime_error("failed to present!");
		
		deviceData.PresentQueue.waitIdle();
		
		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
}
