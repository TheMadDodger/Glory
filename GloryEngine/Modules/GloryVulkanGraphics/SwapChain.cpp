#include "SwapChain.h"
#include <Window.h>
#include "Device.h"
#include "VulkanGraphicsModule.h"
#include "VulkanExceptions.h"

namespace Glory
{
	SwapChain::SwapChain() : m_pWindow(nullptr), m_pDevice(nullptr), m_SwapChain(nullptr)
	{
	}

	SwapChain::~SwapChain()
	{
        const LogicalDeviceData& deviceData = m_pDevice->GetLogicalDeviceData();
        deviceData.LogicalDevice.destroySwapchainKHR(m_SwapChain);

        for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
        {
            deviceData.LogicalDevice.destroyImageView(m_SwapChainImageViews[i]);
        }
        m_SwapChainImageViews.clear();
        m_SwapChainImages.clear();
	}

	void SwapChain::Initialize(VulkanGraphicsModule* pGraphicsModule, Window* pWindow, Device* pDevice)
	{
        m_pWindow = pWindow;
        m_pDevice = pDevice;

        const SwapChainSupportDetails& details = m_pDevice->GetSwapChainSupportDetails();

        // Choose best format
        vk::SurfaceFormatKHR chosenFormat = GetBestFormat(details);

        // Choose best presentation mode
        vk::PresentModeKHR presentMode = GetBestPresentMode(details);

        // Choose best swap extent
        vk::Extent2D swapExtent = GetSwapExtent(details);

        // Get image count
        uint32_t imageCount = CalculateImageCount(details);

        const QueueFamilyIndices& queueFamilyIndices = m_pDevice->GetQueueFamilyIndices();

        // Create swap chain
        vk::SwapchainCreateInfoKHR swapChainCreateInfo = vk::SwapchainCreateInfoKHR()
            .setSurface(pGraphicsModule->GetSurface())
            .setMinImageCount(imageCount)
            .setImageFormat(chosenFormat.format)
            .setImageColorSpace(chosenFormat.colorSpace)
            .setImageExtent(swapExtent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst); // vk::ImageUsageFlagBits::eTransferDst is used when copying an image to the swap chain, which is required when doing post processing!

        uint32_t queueFamilyIndicesArray[] = { queueFamilyIndices.GraphicsFamily.value(), queueFamilyIndices.PresentFamily.value() };

        if (queueFamilyIndices.GraphicsFamily != queueFamilyIndices.PresentFamily) {
            swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndicesArray;
        }
        else {
            swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
            swapChainCreateInfo.queueFamilyIndexCount = 0; // Optional
            swapChainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        swapChainCreateInfo.preTransform = details.Capabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
        const LogicalDeviceData& deviceData = m_pDevice->GetLogicalDeviceData();
        vk::Result result = deviceData.LogicalDevice.createSwapchainKHR(&swapChainCreateInfo, nullptr, &m_SwapChain);
        if (result != vk::Result::eSuccess)
        {
            throw new VulkanException(result);
            return;
        }

        uint32_t swapChainImageCount = 0;
        result = (vk::Result)vkGetSwapchainImagesKHR((VkDevice)deviceData.LogicalDevice, (VkSwapchainKHR)m_SwapChain, &swapChainImageCount, nullptr);
        if (result != vk::Result::eSuccess)
        {
            throw new VulkanException(result);
            return;
        }
        m_SwapChainImages.resize(swapChainImageCount);
        result = deviceData.LogicalDevice.getSwapchainImagesKHR(m_SwapChain, &swapChainImageCount, m_SwapChainImages.data());
        if (result != vk::Result::eSuccess)
        {
            throw new VulkanException(result);
            return;
        }

        m_SwapChainImageFormat = chosenFormat.format;
        m_SwapChainExtent = swapExtent;

        // Create image views
        CreateImageViews();
	}

    const vk::Extent2D& SwapChain::GetExtent() const
    {
        return m_SwapChainExtent;
    }

    vk::SwapchainKHR& SwapChain::GetSwapChain()
    {
        return m_SwapChain;
    }

    size_t SwapChain::GetImageCount() const
    {
        return m_SwapChainImages.size();
    }

    vk::Image SwapChain::GetSwapChainImage(size_t index)
    {
        index = std::clamp(index, (size_t)0, m_SwapChainImages.size());
        return m_SwapChainImages[index];
    }

    vk::ImageView SwapChain::GetSwapChainImageView(size_t index)
    {
        index = std::clamp(index, (size_t)0, m_SwapChainImageViews.size());
        return m_SwapChainImageViews[index];
    }

    const vk::Format& SwapChain::GetFormat() const
    {
        return m_SwapChainImageFormat;
    }

    vk::SurfaceFormatKHR SwapChain::GetBestFormat(const SwapChainSupportDetails& details)
    {
        vk::SurfaceFormatKHR chosenFormat = details.Formats[0];
        for (const auto& availableFormat : details.Formats)
        {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                chosenFormat = availableFormat;
                break;
            }
        }
        return chosenFormat;
    }

    vk::PresentModeKHR SwapChain::GetBestPresentMode(const SwapChainSupportDetails& details)
    {
        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
        for (const auto& availablePresentMode : details.PresentModes)
        {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox)
            {
                presentMode = vk::PresentModeKHR::eMailbox;
                break;
            }
        }
        return presentMode;
    }

    vk::Extent2D SwapChain::GetSwapExtent(const SwapChainSupportDetails& details)
    {
        vk::Extent2D swapExtent;
        if (details.Capabilities.currentExtent.width != UINT32_MAX) {
            swapExtent = details.Capabilities.currentExtent;
        }
        else
        {
            int width, height;
            m_pWindow->GetDrawableSize(&width, &height);

            swapExtent.width = static_cast<uint32_t>(width);
            swapExtent.height = static_cast<uint32_t>(height);

            // Clamp width and height between the allowed minimum and maximum sizes
            swapExtent.width = std::max(details.Capabilities.minImageExtent.width, std::min(details.Capabilities.maxImageExtent.width, swapExtent.width));
            swapExtent.height = std::max(details.Capabilities.minImageExtent.height, std::min(details.Capabilities.maxImageExtent.height, swapExtent.height));
        }
        return swapExtent;
    }

    uint32_t SwapChain::CalculateImageCount(const SwapChainSupportDetails& details)
    {
        uint32_t imageCount = details.Capabilities.minImageCount + 1;
        if (details.Capabilities.minImageCount > 0 && imageCount > details.Capabilities.maxImageCount)
            imageCount = details.Capabilities.maxImageCount;
        return imageCount;
    }

    void SwapChain::CreateImageViews()
    {
        const LogicalDeviceData& deviceData = m_pDevice->GetLogicalDeviceData();
        m_SwapChainImageViews.resize(m_SwapChainImages.size());

        for (size_t i = 0; i < m_SwapChainImages.size(); i++)
        {
            vk::ComponentMapping componentMapping;
            componentMapping.r = vk::ComponentSwizzle::eIdentity;
            componentMapping.g = vk::ComponentSwizzle::eIdentity;
            componentMapping.b = vk::ComponentSwizzle::eIdentity;
            componentMapping.a = vk::ComponentSwizzle::eIdentity;

            vk::ImageSubresourceRange subResourceRange = vk::ImageSubresourceRange()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            vk::ImageViewCreateInfo imageViewCreateInfo = vk::ImageViewCreateInfo()
                .setImage(m_SwapChainImages[i])
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(m_SwapChainImageFormat)
                .setComponents(componentMapping)
                .setSubresourceRange(subResourceRange);

            vk::Result result = deviceData.LogicalDevice.createImageView(&imageViewCreateInfo, nullptr, &m_SwapChainImageViews[i]);
            if (result != vk::Result::eSuccess)
            {
                throw new VulkanException(result);
                return;
            }
        }
    }
}