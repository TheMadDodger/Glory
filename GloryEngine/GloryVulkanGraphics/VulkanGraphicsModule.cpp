#include "VulkanGraphicsModule.h"
#include <Game.h>
#include <Engine.h>
#include <iostream>
#include "VulkanExceptions.h"
#include "Device.h"
#include "SwapChain.h"
#include <fstream>
#include "VulkanBuffer.h"
#include <VertexHelpers.h>
#include <chrono>
#include <ImageLoaderModule.h>

namespace Glory
{
	VulkanGraphicsModule::VulkanGraphicsModule() : m_Extensions(std::vector<const char*>()), m_Layers(std::vector<const char*>()), m_AvailableExtensions(std::vector<VkExtensionProperties>()), m_Instance(nullptr),
        m_cInstance(nullptr), m_Surface(nullptr), m_cSurface(VK_NULL_HANDLE), m_pMainWindow(nullptr), m_pDeviceManager(nullptr), m_pSwapChain(nullptr)
	{
	}

	VulkanGraphicsModule::~VulkanGraphicsModule()
	{
	}

    VkSurfaceKHR VulkanGraphicsModule::GetCSurface()
    {
        return m_cSurface;
    }

    vk::SurfaceKHR VulkanGraphicsModule::GetSurface()
    {
        return m_Surface;
    }

    VkInstance VulkanGraphicsModule::GetCInstance()
    {
        return m_cInstance;
    }

    vk::Instance VulkanGraphicsModule::GetInstance()
    {
        return m_Instance;
    }

    VulkanDeviceManager* VulkanGraphicsModule::GetDeviceManager()
    {
        return m_pDeviceManager;
    }

    const std::vector<const char*>& VulkanGraphicsModule::GetExtensions() const
    {
        return m_Extensions;
    }

    const std::vector<const char*>& VulkanGraphicsModule::GetValidationLayers() const
    {
        return m_Layers;
    }

    Buffer* VulkanGraphicsModule::CreateVertexBuffer_Internal(uint32_t bufferSize)
    {
        return nullptr;
    }

	void VulkanGraphicsModule::Initialize()
	{
        // Get the required extensions from the window
        m_pMainWindow = Game::GetGame().GetEngine()->GetWindowModule()->GetMainWindow();
		m_pMainWindow->GetVulkanRequiredExtensions(m_Extensions);

        // Use validation layers if this is a debug build
#if defined(_DEBUG)
        InitializeValidationLayers();
#endif

        CreateVulkanInstance();
        GetAvailableExtensions();
        CreateSurface();
        LoadPhysicalDevices();
        CreateLogicalDevice();
        CreateSwapChain();

        // Create render pass
        vk::AttachmentDescription colorAttachment = vk::AttachmentDescription()
            .setFormat(m_pSwapChain->GetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        vk::AttachmentReference colorAttachmentRef = vk::AttachmentReference()
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDescription subPass = vk::SubpassDescription()
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&colorAttachmentRef);

        vk::SubpassDependency dependancy = vk::SubpassDependency()
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask((vk::AccessFlags)0)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

        vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo()
            .setAttachmentCount(1)
            .setPAttachments(&colorAttachment)
            .setSubpassCount(1)
            .setPSubpasses(&subPass)
            .setDependencyCount(1)
            .setPDependencies(&dependancy);

        auto deviceData = m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
        m_RenderPass = deviceData.LogicalDevice.createRenderPass(renderPassCreateInfo);
        if (m_RenderPass == nullptr)
        {
            throw std::runtime_error("failed to create render pass!");
        }

        // Create texture
        ImageLoaderModule* pImageLoader = Game::GetGame().GetEngine()->GetModule<ImageLoaderModule>();
        ImageData* pImageData = (ImageData*)pImageLoader->Load("./Resources/chubz.PNG");

        SamplerSettings samplerSettings = SamplerSettings();
        samplerSettings.MagFilter = Filter::F_Linear;
        samplerSettings.MinFilter = Filter::F_Linear;
        samplerSettings.AddressModeU = SamplerAddressMode::SAM_Repeat;
        samplerSettings.AddressModeV = SamplerAddressMode::SAM_Repeat;
        samplerSettings.AddressModeW = SamplerAddressMode::SAM_Repeat;
        samplerSettings.AnisotropyEnable = true;
        samplerSettings.MaxAnisotropy = m_pDeviceManager->GetSelectedDevice()->GetDeviceProperties().limits.maxSamplerAnisotropy;
        samplerSettings.UnnormalizedCoordinates = false;
        samplerSettings.CompareEnable = false;
        samplerSettings.CompareOp = CompareOp::OP_Always;
        samplerSettings.MipmapMode = Filter::F_Linear;
        samplerSettings.MipLODBias = 0.0f;
        samplerSettings.MinLOD = 0.0f;
        samplerSettings.MaxLOD = 0.0f;

        vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        m_pTexture = new VulkanTexture(pImageData, ImageType::IT_2D, (uint32_t)imageUsageFlags, (uint32_t)vk::SharingMode::eExclusive, ImageAspect::IA_Color, samplerSettings);
        m_pTexture->Create();

        /// Create descriptor set layout
        vk::DescriptorSetLayoutBinding uboLayoutBinding = vk::DescriptorSetLayoutBinding();
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        // For the texture sampler
        vk::DescriptorSetLayoutBinding samplerLayoutBinding = vk::DescriptorSetLayoutBinding();
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

        vk::DescriptorSetLayoutCreateInfo layoutInfo = vk::DescriptorSetLayoutCreateInfo();
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (deviceData.LogicalDevice.createDescriptorSetLayout(&layoutInfo, nullptr, &m_DescriptorSetLayout) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create descriptor set layout!");

        // Create descriptor pool
        std::array<vk::DescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(m_pSwapChain->GetImageCount());

        poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(m_pSwapChain->GetImageCount());

        vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo();
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(m_pSwapChain->GetImageCount());

        if(deviceData.LogicalDevice.createDescriptorPool(&poolInfo, nullptr, &m_DescriptorPool) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create descriptor pool!");

        // Create uniform buffers
        m_pUniformBufers.resize(m_pSwapChain->GetImageCount());
        vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        for (size_t i = 0; i < m_pSwapChain->GetImageCount(); i++)
        {
            m_pUniformBufers[i] = new VulkanBuffer(sizeof(UniformBufferObject), (uint32_t)vk::BufferUsageFlagBits::eUniformBuffer, (uint32_t)memoryFlags);
            m_pUniformBufers[i]->CreateBuffer();
        }

        // Create descriptor sets
        std::vector<vk::DescriptorSetLayout> layouts(m_pSwapChain->GetImageCount(), m_DescriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo();
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_pSwapChain->GetImageCount());
        allocInfo.pSetLayouts = layouts.data();

        m_DescriptorSets.resize(m_pSwapChain->GetImageCount());
        if(deviceData.LogicalDevice.allocateDescriptorSets(&allocInfo, m_DescriptorSets.data()) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to allocate descriptor sets!");

        for (size_t i = 0; i < m_pSwapChain->GetImageCount(); i++)
        {
            vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo();
            bufferInfo.buffer = m_pUniformBufers[i]->GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo();
            imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            imageInfo.imageView = m_pTexture->GetTextureImageView();
            imageInfo.sampler = m_pTexture->GetTextureSampler();

            std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};
            descriptorWrites[0].dstSet = m_DescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            descriptorWrites[0].pImageInfo = nullptr;
            descriptorWrites[0].pTexelBufferView = nullptr;

            descriptorWrites[1].dstSet = m_DescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;
            descriptorWrites[1].pBufferInfo = nullptr;
            descriptorWrites[1].pTexelBufferView = nullptr;

            deviceData.LogicalDevice.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

        /// Create graphics pipeline
        // Load shaders
        auto vertShaderCode = ReadFile("Shaders/texturetest_vert.spv");
        auto fragShaderCode = ReadFile("Shaders/texturetest_frag.spv");

        // Create shader modules
        vk::ShaderModule vertShaderModule;
        vk::ShaderModule fragShaderModule;

        vk::ShaderModuleCreateInfo shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
            .setCodeSize(vertShaderCode.size())
            .setPCode(reinterpret_cast<const uint32_t*>(vertShaderCode.data()));
        vertShaderModule = deviceData.LogicalDevice.createShaderModule(shaderModuleCreateInfo, nullptr);

        shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
            .setCodeSize(fragShaderCode.size())
            .setPCode(reinterpret_cast<const uint32_t*>(fragShaderCode.data()));
        fragShaderModule = deviceData.LogicalDevice.createShaderModule(shaderModuleCreateInfo, nullptr);

        vk::PipelineShaderStageCreateInfo vertShaderStageInfo = vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(vertShaderModule)
            .setPName("main");

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo = vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(fragShaderModule)
            .setPName("main");

        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo , fragShaderStageInfo };

        // Create vertex buffer
        const VertexPosColorTex vertices[] = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };

        const float* verticeArray = (const float*)vertices;

        uint32_t bufferSize = sizeof(VertexPosColorTex) * 4;
        vk::MemoryPropertyFlags stagingFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        VulkanBuffer* pStagingBuffer = new VulkanBuffer(bufferSize, (uint32_t)vk::BufferUsageFlagBits::eTransferSrc, (uint32_t)stagingFlags);
        pStagingBuffer->CreateBuffer();
        pStagingBuffer->Assign(verticeArray);
        
        memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
        VulkanBuffer* pVertexBuffer = new VulkanBuffer(bufferSize, (uint32_t)usageFlags, (uint32_t)memoryFlags);
        pVertexBuffer->CreateBuffer();
        pVertexBuffer->CopyFrom(pStagingBuffer, bufferSize);
        m_pVertexBuffer = pVertexBuffer;
        delete pStagingBuffer;

        const uint16_t indices[] = {
            0, 1, 2, 2, 3, 0
        };

        uint32_t indexBufferSize = sizeof(uint32_t) * 6;
        pStagingBuffer = new VulkanBuffer(indexBufferSize, (uint32_t)vk::BufferUsageFlagBits::eTransferSrc, (uint32_t)stagingFlags);
        pStagingBuffer->CreateBuffer();
        pStagingBuffer->Assign(indices);

        usageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
        VulkanBuffer* pIndexBuffer = new VulkanBuffer(indexBufferSize, (uint32_t)usageFlags, (uint32_t)memoryFlags);
        pIndexBuffer->CreateBuffer();
        pIndexBuffer->CopyFrom(pStagingBuffer, indexBufferSize);
        m_pIndexBuffer = pIndexBuffer;
        delete pStagingBuffer;

        const std::vector<AttributeType> attributeTypes = {
            AttributeType::Float2,
            AttributeType::Float3,
            AttributeType::Float2,
        };

        m_pMesh = new VulkanMesh(4, 6, InputRate::Vertex, 0, sizeof(VertexPosColorTex), attributeTypes);
        m_pMesh->CreateBindingAndAttributeData();

        // Vertex input state
        vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
            .setVertexBindingDescriptionCount(1)
            .setPVertexBindingDescriptions(m_pMesh->GetVertexInputBindingDescription())
            .setVertexAttributeDescriptionCount(static_cast<uint32_t>(m_pMesh->GetVertexInputAttributeDescriptionsCount()))
            .setPVertexAttributeDescriptions(m_pMesh->GetVertexInputAttributeDescriptions());

        // Input assembly
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
            .setTopology(vk::PrimitiveTopology::eTriangleList)
            .setPrimitiveRestartEnable(VK_FALSE);

        auto swapchainExtent = m_pSwapChain->GetExtent();

        // Viewport and scissor
        vk::Viewport viewport = vk::Viewport()
            .setX(0.0f)
            .setY(0.0f)
            .setWidth((float)swapchainExtent.width)
            .setHeight((float)swapchainExtent.height)
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);

        vk::Rect2D scissor = vk::Rect2D()
            .setOffset({ 0,0 })
            .setExtent(swapchainExtent);

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
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
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
        vk::PipelineColorBlendAttachmentState colorBlendAttachmentCreateInfo = vk::PipelineColorBlendAttachmentState()
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
            .setBlendEnable(VK_FALSE)
            .setSrcColorBlendFactor(vk::BlendFactor::eOne)
            .setDstColorBlendFactor(vk::BlendFactor::eZero)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
            .setAlphaBlendOp(vk::BlendOp::eAdd);

        vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
            .setLogicOpEnable(VK_FALSE)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachmentCount(1)
            .setPAttachments(&colorBlendAttachmentCreateInfo)
            .setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

        // Dynamic state
        //vk::DynamicState dynamicStates[] = {
        //    vk::DynamicState::eViewport,
        //    vk::DynamicState::eLineWidth
        //};
        //
        //vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
        //    .setDynamicStateCount(2)
        //    .setPDynamicStates(dynamicStates);

        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(1)
            .setPSetLayouts(&m_DescriptorSetLayout)
            .setPushConstantRangeCount(0)
            .setPPushConstantRanges(nullptr);

        m_PipelineLayout = deviceData.LogicalDevice.createPipelineLayout(pipelineLayoutCreateInfo);
        if (m_PipelineLayout == nullptr)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        // Create the pipeline
        vk::GraphicsPipelineCreateInfo pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
            .setStageCount(2)
            .setPStages(shaderStages)
            .setPVertexInputState(&vertexInputStateCreateInfo)
            .setPInputAssemblyState(&inputAssemblyStateCreateInfo)
            .setPViewportState(&viewportStateCreateInfo)
            .setPRasterizationState(&rasterizationStateCreateInfo)
            .setPMultisampleState(&multisampleStateCreateInfo)
            .setPDepthStencilState(nullptr)
            .setPColorBlendState(&colorBlendStateCreateInfo)
            .setPDynamicState(nullptr)
            .setLayout(m_PipelineLayout)
            .setRenderPass(m_RenderPass)
            .setSubpass(0)
            .setBasePipelineHandle(VK_NULL_HANDLE)
            .setBasePipelineIndex(-1);

        if (deviceData.LogicalDevice.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_GraphicsPipeline) != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        deviceData.LogicalDevice.destroyShaderModule(vertShaderModule);
        deviceData.LogicalDevice.destroyShaderModule(fragShaderModule);

        // Create framebuffers
        m_SwapChainFramebuffers.resize(m_pSwapChain->GetImageCount());
        for (size_t i = 0; i < m_pSwapChain->GetImageCount(); i++)
        {
            vk::ImageView attachments[] = {
                m_pSwapChain->GetSwapChainImageView(i)
            };

            vk::FramebufferCreateInfo frameBufferCreateInfo = vk::FramebufferCreateInfo()
                .setRenderPass(m_RenderPass)
                .setAttachmentCount(1)
                .setPAttachments(attachments)
                .setWidth(swapchainExtent.width)
                .setHeight(swapchainExtent.height)
                .setLayers(1);

            m_SwapChainFramebuffers[i] = deviceData.LogicalDevice.createFramebuffer(frameBufferCreateInfo);
            if (m_SwapChainFramebuffers[i] == nullptr)
                throw std::runtime_error("failed to create framebuffer!");
        }

        //auto queueFamilyIndices = m_pDeviceManager->GetSelectedDevice()->GetQueueFamilyIndices();
        vk::CommandPool commandPool = m_pDeviceManager->GetSelectedDevice()->GetGraphicsCommandPool();

        //m_pVertexBuffer = new VertexBuffer({
        //    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        //    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        //    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        //});

        // Create command buffers
        m_CommandBuffers.resize(m_SwapChainFramebuffers.size());

        vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(commandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount((uint32_t)m_CommandBuffers.size());

        if (deviceData.LogicalDevice.allocateCommandBuffers(&commandBufferAllocateInfo, m_CommandBuffers.data()) != vk::Result::eSuccess)
            throw std::runtime_error("failed to allocate command buffers!");

        for (size_t i = 0; i < m_CommandBuffers.size(); i++)
        {
            // Start the command buffer
            vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo()
                .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
                .setPInheritanceInfo(nullptr);

            if (m_CommandBuffers[i].begin(&commandBufferBeginInfo) != vk::Result::eSuccess)
                throw std::runtime_error("failed to begin recording command buffer!");

            // Start a render pass
            vk::Rect2D renderArea = vk::Rect2D()
                .setOffset({ 0,0 })
                .setExtent(swapchainExtent);

            vk::ClearColorValue clearColorValue = vk::ClearColorValue()
                .setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });

            vk::ClearValue clearColor = vk::ClearValue()
                .setColor(clearColorValue);

            vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
                .setRenderPass(m_RenderPass)
                .setFramebuffer(m_SwapChainFramebuffers[i])
                .setRenderArea(renderArea)
                .setClearValueCount(1)
                .setPClearValues(&clearColor);

            m_CommandBuffers[i].beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
            m_CommandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline);

            vk::Buffer vertexBuffers[] = { pVertexBuffer->GetBuffer() };
            vk::DeviceSize offsets[] = { 0 };
            m_CommandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
            m_CommandBuffers[i].bindIndexBuffer(pIndexBuffer->GetBuffer(), 0, vk::IndexType::eUint16);

            m_CommandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, 1, &m_DescriptorSets[i], 0, nullptr);

            m_CommandBuffers[i].drawIndexed(static_cast<uint32_t>(m_pMesh->GetIndexCount()), 1, 0, 0, 0);
            m_CommandBuffers[i].endRenderPass();
            m_CommandBuffers[i].end();
        }

        // Create sync objects
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_ImagesInFlight.resize(m_pSwapChain->GetImageCount(), VK_NULL_HANDLE);

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

	void VulkanGraphicsModule::Cleanup()
	{
        m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData().LogicalDevice.waitIdle();
        m_Extensions.clear();

        //wdelete m_pVertexBuffer;
        //m_pVertexBuffer = nullptr;

#if defined(_DEBUG)
        VkInstance instance = VkInstance(m_Instance);
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        VkDebugUtilsMessengerEXT debugMessenger = VkDebugUtilsMessengerEXT(m_DebugMessenger);
        if (func != nullptr) {
            func(instance, debugMessenger, nullptr);
        }
#endif

        auto deviceData = m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            deviceData.LogicalDevice.destroySemaphore(m_ImageAvailableSemaphores[i]);
            deviceData.LogicalDevice.destroySemaphore(m_RenderFinishedSemaphores[i]);
            deviceData.LogicalDevice.destroyFence(m_InFlightFences[i]);
        }
        m_ImageAvailableSemaphores.clear();
        m_RenderFinishedSemaphores.clear();
        m_InFlightFences.clear();

        deviceData.LogicalDevice.destroyPipeline(m_GraphicsPipeline);
        deviceData.LogicalDevice.destroyPipelineLayout(m_PipelineLayout);

        for (size_t i = 0; i < m_SwapChainFramebuffers.size(); i++)
        {
            deviceData.LogicalDevice.destroyFramebuffer(m_SwapChainFramebuffers[i]);
        }
        m_SwapChainFramebuffers.clear();
        
        deviceData.LogicalDevice.destroyRenderPass(m_RenderPass);

        delete m_pSwapChain;
        m_pSwapChain = nullptr;

        for (size_t i = 0; i < m_pUniformBufers.size(); i++)
        {
            delete m_pUniformBufers[i];
        }
        m_pUniformBufers.clear();

        VulkanImageSampler::Destroy();
        delete m_pTexture;

        deviceData.LogicalDevice.destroyDescriptorPool(m_DescriptorPool);
        deviceData.LogicalDevice.destroyDescriptorSetLayout(m_DescriptorSetLayout);

        delete m_pMesh;
        m_pMesh = nullptr;

        delete m_pVertexBuffer;
        m_pVertexBuffer = nullptr;

        delete m_pIndexBuffer;
        m_pIndexBuffer = nullptr;

        m_Instance.destroySurfaceKHR(m_Surface);
        delete m_pDeviceManager;
        m_pDeviceManager = nullptr;
        m_Instance.destroy();
	}

	void VulkanGraphicsModule::Update()
	{
	}

	void VulkanGraphicsModule::Draw()
	{
        auto deviceData = m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
        deviceData.LogicalDevice.waitForFences(1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

        // Aqcuire swap chain image
        uint32_t imageIndex;
        deviceData.LogicalDevice.acquireNextImageKHR(m_pSwapChain->GetSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            deviceData.LogicalDevice.waitForFences(1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        // Mark the image as now being in use by this frame
        m_ImagesInFlight[imageIndex] = m_InFlightFences[m_CurrentFrame];

        UpdateUniformBuffer(imageIndex);

        // Submit command buffer
        vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
        vk::SubmitInfo submitInfo = vk::SubmitInfo()
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(waitSemaphores)
            .setPWaitDstStageMask(waitStages)
            .setCommandBufferCount(1)
            .setPCommandBuffers(&m_CommandBuffers[imageIndex])
            .setSignalSemaphoreCount(1)
            .setPSignalSemaphores(signalSemaphores);

        deviceData.LogicalDevice.resetFences(1, &m_InFlightFences[m_CurrentFrame]);

        if(deviceData.GraphicsQueue.submit(1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != vk::Result::eSuccess)
            throw std::runtime_error("failed to submit draw command buffer!");

        vk::SwapchainKHR swapChains[] = { m_pSwapChain->GetSwapChain() };
        vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(signalSemaphores)
            .setSwapchainCount(1)
            .setPSwapchains(swapChains)
            .setPImageIndices(&imageIndex)
            .setPResults(nullptr);

        if(deviceData.PresentQueue.presentKHR(&presentInfo) != vk::Result::eSuccess)
            throw std::runtime_error("failed to present!");

        deviceData.PresentQueue.waitIdle();

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

    void VulkanGraphicsModule::InitializeValidationLayers()
    {
        m_Layers.push_back("VK_LAYER_KHRONOS_validation");
        m_Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // Get supported validation layers
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        std::cout << "Available validation layers:" << std::endl;

        for (const auto& layer : availableLayers)
        {
            std::cout << '\t' << layer.layerName << std::endl;
        }

        std::cout << std::endl;

        for (const char* layer : m_Layers)
        {
            auto it = std::find_if(availableLayers.begin(), availableLayers.end(), [layer](const VkLayerProperties& otherLayer)
            {
                return strcmp(layer, otherLayer.layerName) == 0;
            });

            if (it == availableLayers.end()) std::cout << "The layer: " << layer << " is not available " << std::endl;
        }
    }

    void VulkanGraphicsModule::CreateVulkanInstance()
    {
        // vk::ApplicationInfo allows the programmer to specifiy some basic information about the
        // program, which can be useful for layers and tools to provide more debug information.
        // TEMPORARY
        vk::ApplicationInfo appInfo = vk::ApplicationInfo()
            .setPApplicationName("Vulkan C++ Windowed Program Template")
            .setApplicationVersion(1)
            .setPEngineName("LunarG SDK")
            .setEngineVersion(1)
            .setApiVersion(VK_API_VERSION_1_0);

        // Create debug messenger
#if defined(_DEBUG)
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugCallback;
        debugCreateInfo.pUserData = nullptr; // Optional
#endif

        // vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
        // are needed.
        vk::InstanceCreateInfo instInfo = vk::InstanceCreateInfo()
            .setFlags(vk::InstanceCreateFlags())
            .setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount(static_cast<uint32_t>(m_Extensions.size()))
            .setPpEnabledExtensionNames(m_Extensions.data())
            .setEnabledLayerCount(static_cast<uint32_t>(m_Layers.size()))
#if defined(_DEBUG)
            .setPNext(&debugCreateInfo)
#endif
            .setPpEnabledLayerNames(m_Layers.data());

        // Create the Vulkan instance.
        vk::Result result = vk::createInstance(&instInfo, nullptr, &m_Instance);
        if (result != vk::Result::eSuccess)
            throw new CreateVulkanInstanceException(result);

        m_cInstance = VkInstance(m_Instance);

        // Create debug messenger
#if defined(_DEBUG)
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_cInstance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            VkDebugUtilsMessengerEXT debugMessenger;
            func(m_cInstance, &debugCreateInfo, nullptr, &debugMessenger);
            m_DebugMessenger = vk::DebugUtilsMessengerEXT(debugMessenger);
        }
        else {
            std::cout << "Could not find the create debug messenger function." << std::endl;
        }
#endif
    }

    void VulkanGraphicsModule::GetAvailableExtensions()
    {
        // Check extension support
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        m_AvailableExtensions.resize(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, m_AvailableExtensions.data());

        std::cout << "Available extensions:" << std::endl;
        for (const auto& extension : m_AvailableExtensions)
        {
            std::cout << '\t' << extension.extensionName << std::endl;
        }
    }

    void VulkanGraphicsModule::CreateSurface()
    {
        m_pMainWindow->GetVulkanSurface(m_cInstance, &m_cSurface);
        m_Surface = vk::SurfaceKHR(m_cSurface);
    }

    void VulkanGraphicsModule::LoadPhysicalDevices()
    {
        m_pDeviceManager = new VulkanDeviceManager();
        m_pDeviceManager->Initialize(this);

        // Check for required device extensions
        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        m_pDeviceManager->CheckDeviceSupport(this, deviceExtensions);

        // TEMPORARILY force the chosen device to the second index
        m_pDeviceManager->SelectDevice(1);
    }

    void VulkanGraphicsModule::CreateLogicalDevice()
    {
        m_pDeviceManager->GetSelectedDevice()->CreateLogicalDevice(this);
    }

    void VulkanGraphicsModule::CreateSwapChain()
    {
        m_pSwapChain = new SwapChain(m_pMainWindow, m_pDeviceManager->GetSelectedDevice());
        m_pSwapChain->Initialize(this);
    }

    void VulkanGraphicsModule::UpdateUniformBuffer(uint32_t imageIndex)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        auto extent = m_pSwapChain->GetExtent();
        ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan
        m_pUniformBufers[imageIndex]->Assign(&ubo);

    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanGraphicsModule::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        if (messageSeverity != VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT && messageSeverity != VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) return VK_FALSE;
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    vk::CommandBuffer VulkanGraphicsModule::BeginSingleTimeCommands()
    {
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        vk::CommandPool commandPool = pDevice->GetGraphicsCommandPool();

        vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo();
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();
        if (deviceData.LogicalDevice.allocateCommandBuffers(&allocInfo, &commandBuffer) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to allocate command buffer!");

        vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        commandBuffer.begin(beginInfo);
        return commandBuffer;
    }

    void VulkanGraphicsModule::EndSingleTimeCommands(vk::CommandBuffer commandBuffer)
    {
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        vk::CommandPool commandPool = pDevice->GetGraphicsCommandPool();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        commandBuffer.end();

        vk::SubmitInfo submitInfo = vk::SubmitInfo();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        deviceData.GraphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
        deviceData.GraphicsQueue.waitIdle();

        deviceData.LogicalDevice.freeCommandBuffers(commandPool, 1, &commandBuffer);
    }

    void VulkanGraphicsModule::TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
    {
        vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

        vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier();
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;

        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.srcAccessMask = (vk::AccessFlags)0;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else {
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

    std::vector<char> VulkanGraphicsModule::ReadFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }
}
