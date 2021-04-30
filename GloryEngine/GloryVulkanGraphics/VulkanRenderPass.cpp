#include "VulkanRenderPass.h"
#include "VulkanGraphicsModule.h"
#include "VulkanDeviceManager.h"
#include "Device.h"
#include <Game.h>

namespace Glory
{
    VulkanRenderPass::VulkanRenderPass(const RenderPassCreateInfo& createInfo) : m_CreateInfo(createInfo)
    {
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        for (size_t i = 0; i < m_SwapChainFramebuffers.size(); i++)
        {
            deviceData.LogicalDevice.destroyFramebuffer(m_SwapChainFramebuffers[i]);
        }
        m_SwapChainFramebuffers.clear();

        deviceData.LogicalDevice.destroyRenderPass(m_RenderPass);
    }

    void VulkanRenderPass::Initialize()
    {
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        // Create render pass
        vk::AttachmentDescription colorAttachment = vk::AttachmentDescription()
            .setFormat(m_CreateInfo.Format)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        vk::AttachmentDescription depthAttachment = vk::AttachmentDescription()
            .setFormat(m_CreateInfo.pDepth->m_DepthFormat)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::AttachmentReference colorAttachmentRef = vk::AttachmentReference()
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::AttachmentReference depthAttachmentRef = vk::AttachmentReference()
            .setAttachment(1)
            .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::SubpassDescription subPass = vk::SubpassDescription()
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&colorAttachmentRef)
            .setPDepthStencilAttachment(&depthAttachmentRef);

        vk::SubpassDependency dependancy = vk::SubpassDependency()
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
            .setSrcAccessMask((vk::AccessFlags)0)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

        std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

        vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo()
            .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
            .setPAttachments(attachments.data())
            .setSubpassCount(1)
            .setPSubpasses(&subPass)
            .setDependencyCount(1)
            .setPDependencies(&dependancy);

        m_RenderPass = deviceData.LogicalDevice.createRenderPass(renderPassCreateInfo);
        if (m_RenderPass == nullptr)
        {
            throw std::runtime_error("failed to create render pass!");
        }

        CreateSwapChainFrameBuffers();
    }

    void VulkanRenderPass::CreateSwapChainFrameBuffers()
    {
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        auto swapchainExtent = m_CreateInfo.Extent;

        // Create framebuffers
        m_SwapChainFramebuffers.resize(m_CreateInfo.ImageViews.size());
        for (size_t i = 0; i < m_CreateInfo.ImageViews.size(); i++)
        {
            vk::ImageView attachments[] = {
                m_CreateInfo.ImageViews[i],
                m_CreateInfo.pDepth->m_DepthImageView,
            };

            vk::FramebufferCreateInfo frameBufferCreateInfo = vk::FramebufferCreateInfo()
                .setRenderPass(m_RenderPass)
                .setAttachmentCount(2)
                .setPAttachments(attachments)
                .setWidth(swapchainExtent.width)
                .setHeight(swapchainExtent.height)
                .setLayers(1);

            m_SwapChainFramebuffers[i] = deviceData.LogicalDevice.createFramebuffer(frameBufferCreateInfo);
            if (m_SwapChainFramebuffers[i] == nullptr)
                throw std::runtime_error("failed to create framebuffer!");
        }
    }

    RenderPassCreateInfo::RenderPassCreateInfo() : HasDepth(false), pDepth(nullptr)
    {

    }
}