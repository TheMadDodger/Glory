#include "DepthImage.h"
#include "VulkanGraphicsModule.h"
#include "VulkanDeviceManager.h"
#include "Device.h"
#include <Game.h>

namespace Glory
{
    DepthImage::DepthImage(SwapChain* pSwapChain) : m_pSwapChain(pSwapChain)
    {
    }

    DepthImage::~DepthImage()
    {
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        deviceData.LogicalDevice.destroyImageView(m_DepthImageView, nullptr);
        deviceData.LogicalDevice.destroyImage(m_DepthImage, nullptr);
        deviceData.LogicalDevice.freeMemory(m_DepthImageMemory, nullptr);
    }

    void DepthImage::Initialize()
	{
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        // Create depth resources
        m_DepthFormat = pDevice->FindDepthFormat();
        auto extent = m_pSwapChain->GetExtent();

        VulkanGraphicsModule::CreateImage(extent.width, extent.height, m_DepthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, m_DepthImage, m_DepthImageMemory);
        m_DepthImageView = VulkanGraphicsModule::CreateImageView(m_DepthImage, m_DepthFormat, vk::ImageAspectFlagBits::eDepth);
        VulkanGraphicsModule::TransitionImageLayout(m_DepthImage, m_DepthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageAspectFlagBits::eDepth);
	}
}
