#include "DeferredRenderPassTest.h"
//#include "VulkanDeviceManager.h"
//#include "Device.h"
//#include "VulkanExceptions.h"
//
//#include <Engine.h>
//
//namespace Glory
//{
//    DeferredRenderPassTest::DeferredRenderPassTest(const RenderPassCreateInfo& createInfo) : m_CreateInfo(createInfo)
//    {
//    }
//
//    DeferredRenderPassTest::~DeferredRenderPassTest()
//    {
//        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
//        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
//        Device* pDevice = pDeviceManager->GetSelectedDevice();
//        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();
//
//
//        for (size_t i = 0; i < m_CreateInfo.SwapChainImageCount; i++)
//        {
//            deviceData.LogicalDevice.destroyFramebuffer(m_Framebuffers[i]);
//
//            deviceData.LogicalDevice.destroyImageView(m_PositionImageViews[i]);
//            deviceData.LogicalDevice.destroyImage(m_PositionImages[i]);
//            deviceData.LogicalDevice.freeMemory(m_PositionImagesMemory[i]);
//
//            deviceData.LogicalDevice.destroyImageView(m_ColorImageViews[i]);
//            deviceData.LogicalDevice.destroyImage(m_ColorImages[i]);
//            deviceData.LogicalDevice.freeMemory(m_ColorImagesMemory[i]);
//
//            deviceData.LogicalDevice.destroyImageView(m_NormalImageViews[i]);
//            deviceData.LogicalDevice.destroyImage(m_NormalImages[i]);
//            deviceData.LogicalDevice.freeMemory(m_NormalImagesMemory[i]);
//        }
//
//        deviceData.LogicalDevice.destroyRenderPass(m_RenderPass);
//    }
//
//    void DeferredRenderPassTest::Initialize()
//    {
//        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
//        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
//        Device* pDevice = pDeviceManager->GetSelectedDevice();
//        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();
//
//        // Create render pass
//        vk::AttachmentDescription baseColorAttachment = vk::AttachmentDescription()
//            .setFormat(m_CreateInfo.Format)
//            .setSamples(vk::SampleCountFlagBits::e1)
//            .setLoadOp(vk::AttachmentLoadOp::eClear)
//            .setStoreOp(vk::AttachmentStoreOp::eStore)
//            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
//            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
//            .setInitialLayout(vk::ImageLayout::eUndefined)
//            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
//
//        vk::AttachmentDescription positionAttachment = vk::AttachmentDescription()
//            .setFormat(m_CreateInfo.Format)
//            .setSamples(vk::SampleCountFlagBits::e1)
//            .setLoadOp(vk::AttachmentLoadOp::eClear)
//            .setStoreOp(vk::AttachmentStoreOp::eStore)
//            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
//            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
//            .setInitialLayout(vk::ImageLayout::eUndefined)
//            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
//
//        vk::AttachmentDescription normalsAttachment = vk::AttachmentDescription()
//            .setFormat(m_CreateInfo.Format)
//            .setSamples(vk::SampleCountFlagBits::e1)
//            .setLoadOp(vk::AttachmentLoadOp::eClear)
//            .setStoreOp(vk::AttachmentStoreOp::eStore)
//            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
//            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
//            .setInitialLayout(vk::ImageLayout::eUndefined)
//            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
//
//        vk::AttachmentDescription depthAttachment = vk::AttachmentDescription()
//            .setFormat(m_CreateInfo.pDepth->GetFormat())
//            .setSamples(vk::SampleCountFlagBits::e1)
//            .setLoadOp(vk::AttachmentLoadOp::eClear)
//            .setStoreOp(vk::AttachmentStoreOp::eDontCare)
//            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
//            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
//            .setInitialLayout(vk::ImageLayout::eUndefined)
//            .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
//
//        vk::AttachmentReference colorAttachmentRefs[3] = {};
//        colorAttachmentRefs[0].setAttachment(0)
//            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
//        colorAttachmentRefs[1].setAttachment(1)
//            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
//        colorAttachmentRefs[2].setAttachment(2)
//            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
//
//        vk::AttachmentReference depthAttachmentRef = vk::AttachmentReference()
//            .setAttachment(3)
//            .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
//
//        vk::SubpassDescription subPass = vk::SubpassDescription()
//            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
//            .setColorAttachmentCount(3)
//            .setPColorAttachments(colorAttachmentRefs)
//            .setPDepthStencilAttachment(&depthAttachmentRef);
//
//        vk::SubpassDependency dependancy = vk::SubpassDependency()
//            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
//            .setDstSubpass(0)
//            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
//            .setSrcAccessMask((vk::AccessFlags)0)
//            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
//            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
//
//        std::array<vk::AttachmentDescription, 4> attachments = { baseColorAttachment, positionAttachment, normalsAttachment, depthAttachment };
//
//        vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo()
//            .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
//            .setPAttachments(attachments.data())
//            .setSubpassCount(1)
//            .setPSubpasses(&subPass)
//            .setDependencyCount(1)
//            .setPDependencies(&dependancy);
//
//        m_RenderPass = deviceData.LogicalDevice.createRenderPass(renderPassCreateInfo);
//        if (m_RenderPass == nullptr)
//        {
//            throw std::runtime_error("failed to create render pass!");
//        }
//
//        CreateImages();
//        CreateImageViews();
//        CreateFrameBuffers();
//    }
//
//    void DeferredRenderPassTest::CreateImages()
//    {
//        m_PositionImages.resize(m_CreateInfo.SwapChainImageCount);
//        m_PositionImagesMemory.resize(m_CreateInfo.SwapChainImageCount);
//
//        m_ColorImages.resize(m_CreateInfo.SwapChainImageCount);
//        m_ColorImagesMemory.resize(m_CreateInfo.SwapChainImageCount);
//
//        m_NormalImages.resize(m_CreateInfo.SwapChainImageCount);
//        m_NormalImagesMemory.resize(m_CreateInfo.SwapChainImageCount);
//
//        for (size_t i = 0; i < m_CreateInfo.SwapChainImageCount; i++)
//        {
//            VulkanGraphicsModule::CreateImage(m_CreateInfo.Extent.width, m_CreateInfo.Extent.height, m_CreateInfo.Format, vk::ImageTiling::eOptimal,
//                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eDeviceLocal, m_PositionImages[i], m_PositionImagesMemory[i]);
//            VulkanGraphicsModule::CreateImage(m_CreateInfo.Extent.width, m_CreateInfo.Extent.height, m_CreateInfo.Format, vk::ImageTiling::eOptimal,
//                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eDeviceLocal, m_ColorImages[i], m_ColorImagesMemory[i]);
//            VulkanGraphicsModule::CreateImage(m_CreateInfo.Extent.width, m_CreateInfo.Extent.height, m_CreateInfo.Format, vk::ImageTiling::eOptimal,
//                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eDeviceLocal, m_NormalImages[i], m_NormalImagesMemory[i]);
//        }
//    }
//
//    void DeferredRenderPassTest::CreateImageViews()
//    {
//        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
//        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
//        Device* pDevice = pDeviceManager->GetSelectedDevice();
//        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();
//
//        m_PositionImageViews.resize(m_CreateInfo.SwapChainImageCount);
//        m_ColorImageViews.resize(m_CreateInfo.SwapChainImageCount);
//        m_NormalImageViews.resize(m_CreateInfo.SwapChainImageCount);
//
//        for (size_t i = 0; i < m_CreateInfo.SwapChainImageCount; i++)
//        {
//            vk::ComponentMapping componentMapping;
//            componentMapping.r = vk::ComponentSwizzle::eIdentity;
//            componentMapping.g = vk::ComponentSwizzle::eIdentity;
//            componentMapping.b = vk::ComponentSwizzle::eIdentity;
//            componentMapping.a = vk::ComponentSwizzle::eIdentity;
//
//            vk::ImageSubresourceRange subResourceRange = vk::ImageSubresourceRange()
//                .setAspectMask(vk::ImageAspectFlagBits::eColor)
//                .setBaseMipLevel(0)
//                .setLevelCount(1)
//                .setBaseArrayLayer(0)
//                .setLayerCount(1);
//
//            vk::ImageViewCreateInfo imageViewCreateInfo = vk::ImageViewCreateInfo()
//                .setImage(m_PositionImages[i])
//                .setViewType(vk::ImageViewType::e2D)
//                .setFormat(m_CreateInfo.Format)
//                .setComponents(componentMapping)
//                .setSubresourceRange(subResourceRange);
//
//            vk::Result result = deviceData.LogicalDevice.createImageView(&imageViewCreateInfo, nullptr, &m_PositionImageViews[i]);
//            if (result != vk::Result::eSuccess)
//            {
//                throw new VulkanException(result);
//                return;
//            }
//
//            imageViewCreateInfo.setImage(m_ColorImages[i]);
//
//            result = deviceData.LogicalDevice.createImageView(&imageViewCreateInfo, nullptr, &m_ColorImageViews[i]);
//            if (result != vk::Result::eSuccess)
//            {
//                throw new VulkanException(result);
//                return;
//            }
//
//            imageViewCreateInfo.setImage(m_NormalImages[i]);
//
//            result = deviceData.LogicalDevice.createImageView(&imageViewCreateInfo, nullptr, &m_NormalImageViews[i]);
//            if (result != vk::Result::eSuccess)
//            {
//                throw new VulkanException(result);
//                return;
//            }
//        }
//    }
//
//    void DeferredRenderPassTest::CreateFrameBuffers()
//    {
//        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
//        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
//        Device* pDevice = pDeviceManager->GetSelectedDevice();
//        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();
//
//        m_Framebuffers.resize(m_CreateInfo.SwapChainImageCount);
//
//        for (size_t i = 0; i < m_CreateInfo.SwapChainImageCount; i++)
//        {
//            vk::ImageView attachments[] = {
//                m_ColorImageViews[i],
//                m_PositionImageViews[i],
//                m_NormalImageViews[i],
//                m_CreateInfo.pDepth->GetImageView(),
//            };
//                
//            vk::FramebufferCreateInfo frameBufferCreateInfo = vk::FramebufferCreateInfo()
//                .setRenderPass(m_RenderPass)
//                .setAttachmentCount(4)
//                .setPAttachments(attachments)
//                .setWidth(m_CreateInfo.Extent.width)
//                .setHeight(m_CreateInfo.Extent.height)
//                .setLayers(1);
//                
//            m_Framebuffers[i] = deviceData.LogicalDevice.createFramebuffer(frameBufferCreateInfo);
//            if (m_Framebuffers[i] == nullptr)
//                throw std::runtime_error("failed to create framebuffer!");
//        }
//    }
//}
