#include "DepthImage.h"
#include "VulkanGraphicsModule.h"
#include "VulkanDeviceManager.h"
#include "Device.h"

#include <Engine.h>

namespace Glory
{
    DepthImage::DepthImage():
        m_pGraphics(nullptr), m_Extent()
    {
    }

    DepthImage::~DepthImage()
    {
        VulkanDeviceManager& deviceManager = m_pGraphics->GetDeviceManager();
        Device* pDevice = deviceManager.GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        deviceData.LogicalDevice.destroyImageView(m_DepthImageView, nullptr);
        deviceData.LogicalDevice.destroyImage(m_DepthImage, nullptr);
        deviceData.LogicalDevice.freeMemory(m_DepthImageMemory, nullptr);
    }

    const vk::ImageView& DepthImage::GetImageView() const
    {
        return m_DepthImageView;
    }

    const vk::Format& DepthImage::GetFormat() const
    {
        return m_DepthFormat;
    }

    void DepthImage::Initialize(VulkanGraphicsModule* pGraphics, vk::Extent2D extent)
	{
        m_pGraphics = pGraphics;
        m_Extent = extent;

        VulkanDeviceManager& deviceManager = m_pGraphics->GetDeviceManager();
        Device* pDevice = deviceManager.GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        /* Create depth resources */
        m_DepthFormat = pDevice->FindDepthFormat();
        m_pGraphics->CreateImage(m_Extent.width, m_Extent.height, m_DepthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, m_DepthImage, m_DepthImageMemory);
        m_DepthImageView = m_pGraphics->CreateImageView(m_DepthImage, m_DepthFormat, vk::ImageAspectFlagBits::eDepth);
        m_pGraphics->TransitionImageLayout(m_DepthImage, m_DepthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageAspectFlagBits::eDepth);
	}
}
