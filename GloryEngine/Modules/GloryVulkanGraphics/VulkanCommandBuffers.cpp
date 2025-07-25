#include "VulkanCommandBuffers.h"
#include "Device.h"
#include "VulkanGraphicsModule.h"

namespace Glory
{
	VulkanCommandBuffers::VulkanCommandBuffers(VulkanGraphicsModule* pModule) : m_pModule(pModule)
	{
	}

	VulkanCommandBuffers::~VulkanCommandBuffers()
	{
        VulkanDeviceManager& deviceManager = m_pModule->GetDeviceManager();
        auto deviceData = deviceManager.GetSelectedDevice()->GetLogicalDeviceData();
        vk::CommandPool commandPool = deviceManager.GetSelectedDevice()->GetGraphicsCommandPool(m_FrameCommandPoolFlags);
        deviceData.LogicalDevice.freeCommandBuffers(commandPool, m_FrameCommandBuffers.size(), m_FrameCommandBuffers.data());
	}

	void VulkanCommandBuffers::Initialize()
	{
        VulkanDeviceManager& deviceManager = m_pModule->GetDeviceManager();
        auto deviceData = deviceManager.GetSelectedDevice()->GetLogicalDeviceData();
        vk::CommandPool commandPool = deviceManager.GetSelectedDevice()->GetGraphicsCommandPool(m_FrameCommandPoolFlags);

        SwapChain& swapChain = m_pModule->GetSwapChain();

        // Create command buffers
        m_FrameCommandBuffers.resize(swapChain.GetImageCount());

        vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(commandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount((uint32_t)m_FrameCommandBuffers.size());

        if (deviceData.LogicalDevice.allocateCommandBuffers(&commandBufferAllocateInfo, m_FrameCommandBuffers.data()) != vk::Result::eSuccess)
            throw std::runtime_error("failed to allocate command buffers!");
	}

    vk::CommandBuffer VulkanCommandBuffers::GetCurrentFrameCommandBuffer()
    {
        const size_t currentFrameIndex = m_pModule->CurrentImageIndex();
        return m_FrameCommandBuffers[currentFrameIndex];
    }
}