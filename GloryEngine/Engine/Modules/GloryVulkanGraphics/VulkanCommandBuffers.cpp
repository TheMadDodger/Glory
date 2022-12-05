#include "VulkanCommandBuffers.h"
#include "Device.h"
#include "VulkanFrameStates.h"
#include "VulkanGraphicsModule.h"

namespace Glory
{
	VulkanCommandBuffers::VulkanCommandBuffers(VulkanGraphicsModule* pModule) : m_pModule(pModule)
	{
	}

	VulkanCommandBuffers::~VulkanCommandBuffers()
	{
        auto pDeviceManager = m_pModule->GetDeviceManager();
        auto deviceData = pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
        vk::CommandPool commandPool = pDeviceManager->GetSelectedDevice()->GetGraphicsCommandPool(m_FrameCommandPoolFlags);
        deviceData.LogicalDevice.freeCommandBuffers(commandPool, m_FrameCommandBuffers.size(), m_FrameCommandBuffers.data());
	}

	void VulkanCommandBuffers::Initialize()
	{
        auto pDeviceManager = m_pModule->GetDeviceManager();
        auto deviceData = pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
        vk::CommandPool commandPool = pDeviceManager->GetSelectedDevice()->GetGraphicsCommandPool(m_FrameCommandPoolFlags);

        SwapChain* pSwapChain = m_pModule->GetSwapChain();

        // Create command buffers
        m_FrameCommandBuffers.resize(pSwapChain->GetImageCount());

        vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(commandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount((uint32_t)m_FrameCommandBuffers.size());

        if (deviceData.LogicalDevice.allocateCommandBuffers(&commandBufferAllocateInfo, m_FrameCommandBuffers.data()) != vk::Result::eSuccess)
            throw std::runtime_error("failed to allocate command buffers!");
	}

    vk::CommandBuffer VulkanCommandBuffers::GetCurrentFrameCommandBuffer()
    {
        size_t currentFrameIndex = VulkanFrameStates::GetCurrentImageIndex();
        return m_FrameCommandBuffers[currentFrameIndex];
    }
}