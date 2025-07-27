#include "DeferredPipelineTest.h"
//#include "VulkanGraphicsModule.h"
//#include "Device.h"
//#include <FileLoaderModule.h>
//#include <iostream>
//#include <Game.h>
//#include <Engine.h>
//
//namespace Glory
//{
//    DeferredPipelineTest::DeferredPipelineTest(VulkanMesh* pMesh, DeferredRenderPassTest* pRenderPass, VulkanTexture* pTexture, const vk::Extent2D& extent) : m_pMesh(pMesh), m_pRenderPass(pRenderPass), m_pTexture(pTexture), m_Extent(extent)
//	{
//	}
//
//	DeferredPipelineTest::~DeferredPipelineTest()
//	{
//        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
//        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
//        Device* pDevice = pDeviceManager->GetSelectedDevice();
//        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();
//
//        deviceData.LogicalDevice.destroyPipeline(m_GraphicsPipeline);
//        deviceData.LogicalDevice.destroyPipelineLayout(m_PipelineLayout);
//
//        for (size_t i = 0; i < m_DescriptorSetLayouts.size(); i++)
//        {
//            deviceData.LogicalDevice.destroyDescriptorSetLayout(m_DescriptorSetLayouts[i]);
//        }
//
//        m_DescriptorSetLayouts.clear();
//	}
//
//	void DeferredPipelineTest::Initialize()
//	{
//        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
//        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
//        Device* pDevice = pDeviceManager->GetSelectedDevice();
//        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();
//
//        // Load shaders
//        FileLoaderModule* pFileLoader = Game::GetGame().GetEngine()->GetModule<FileLoaderModule>();
//        FileImportSettings importSettings{};
//        importSettings.Flags = std::ios::ate | std::ios::binary;
//        FileData* pVertFileData = (FileData*)pFileLoader->Load("./Shaders/deferredtest_vert.spv", importSettings);
//        FileData* pFragFileData = (FileData*)pFileLoader->Load("./Shaders/deferredtest_frag.spv", importSettings);
//
//        // Create vulkan shaders
//        VulkanShader* pVertShader = new VulkanShader(pVertFileData, ShaderType::ST_Vertex, "main");
//        pVertShader->Initialize();
//        VulkanShader* pFragShader = new VulkanShader(pFragFileData, ShaderType::ST_Fragment, "main");
//        pFragShader->Initialize();
//
//        m_pShaders.push_back(pVertShader);
//        m_pShaders.push_back(pFragShader);
//
//        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(m_pShaders.size());
//        for (size_t i = 0; i < shaderStages.size(); i++)
//        {
//            shaderStages[i] = m_pShaders[i]->m_PipelineShaderStageInfo;
//        }
//
//        /// Create descriptor set layout
//        vk::DescriptorSetLayoutBinding uboLayoutBinding = vk::DescriptorSetLayoutBinding();
//        uboLayoutBinding.binding = 0;
//        uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
//        uboLayoutBinding.descriptorCount = 1;
//        uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
//        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
//
//        // For the texture sampler
//        vk::DescriptorSetLayoutBinding samplerLayoutBinding = vk::DescriptorSetLayoutBinding();
//        samplerLayoutBinding.binding = 1;
//        samplerLayoutBinding.descriptorCount = 1;
//        samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
//        samplerLayoutBinding.pImmutableSamplers = nullptr;
//        samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
//
//        std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
//
//        vk::DescriptorSetLayoutCreateInfo layoutInfo = vk::DescriptorSetLayoutCreateInfo();
//        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
//        layoutInfo.pBindings = bindings.data();
//
//        m_DescriptorSetLayoutInfos.push_back(layoutInfo);
//
//        // Create descriptor set layout
//        m_DescriptorSetLayouts.resize(m_DescriptorSetLayoutInfos.size());
//
//        for (size_t i = 0; i < m_DescriptorSetLayoutInfos.size(); i++)
//        {
//            if (deviceData.LogicalDevice.createDescriptorSetLayout(&m_DescriptorSetLayoutInfos[0], nullptr, &m_DescriptorSetLayouts[0]) != vk::Result::eSuccess)
//                throw std::runtime_error("Failed to create descriptor set layout!");
//        }
//
//        CreateDescriptorSets();
//
//        // Vertex input state
//        vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
//            .setVertexBindingDescriptionCount(1)
//            .setPVertexBindingDescriptions(m_pMesh->GetVertexInputBindingDescription())
//            .setVertexAttributeDescriptionCount(static_cast<uint32_t>(m_pMesh->GetVertexInputAttributeDescriptionsCount()))
//            .setPVertexAttributeDescriptions(m_pMesh->GetVertexInputAttributeDescriptions());
//
//        // Input assembly
//        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
//            .setTopology(vk::PrimitiveTopology::eTriangleList)
//            .setPrimitiveRestartEnable(VK_FALSE);
//
//        // Viewport and scissor
//        vk::Viewport viewport = vk::Viewport()
//            .setX(0.0f)
//            .setY(0.0f)
//            .setWidth((float)m_Extent.width)
//            .setHeight((float)m_Extent.height)
//            .setMinDepth(0.0f)
//            .setMaxDepth(1.0f);
//
//        vk::Rect2D scissor = vk::Rect2D()
//            .setOffset({ 0,0 })
//            .setExtent(m_Extent);
//
//        vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo()
//            .setViewportCount(1)
//            .setPViewports(&viewport)
//            .setScissorCount(1)
//            .setPScissors(&scissor);
//
//        // Rasterizer state
//        vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo()
//            .setDepthClampEnable(VK_FALSE) // Requires a GPU feature
//            .setRasterizerDiscardEnable(VK_FALSE)
//            .setPolygonMode(vk::PolygonMode::eFill)
//            .setLineWidth(1.0f)
//            .setCullMode(vk::CullModeFlagBits::eBack)
//            .setFrontFace(vk::FrontFace::eCounterClockwise)
//            .setDepthBiasEnable(VK_FALSE)
//            .setDepthBiasConstantFactor(0.0f)
//            .setDepthBiasClamp(0.0f)
//            .setDepthBiasSlopeFactor(0.0f);
//
//        // Multisampling state
//        vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo()
//            .setSampleShadingEnable(VK_FALSE)
//            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
//            .setMinSampleShading(1.0f)
//            .setPSampleMask(nullptr)
//            .setAlphaToCoverageEnable(VK_FALSE)
//            .setAlphaToOneEnable(VK_FALSE);
//
//        // Blend state
//        vk::PipelineColorBlendAttachmentState colorBlendAttachmentCreateInfo[3];
//
//        for (size_t i = 0; i < 3; i++)
//        {
//            colorBlendAttachmentCreateInfo[i] = vk::PipelineColorBlendAttachmentState()
//                .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
//                .setBlendEnable(VK_FALSE)
//                .setSrcColorBlendFactor(vk::BlendFactor::eOne)
//                .setDstColorBlendFactor(vk::BlendFactor::eZero)
//                .setColorBlendOp(vk::BlendOp::eAdd)
//                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
//                .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
//                .setAlphaBlendOp(vk::BlendOp::eAdd);
//        }
//
//        vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
//            .setLogicOpEnable(VK_FALSE)
//            .setLogicOp(vk::LogicOp::eCopy)
//            .setAttachmentCount(3)
//            .setPAttachments(colorBlendAttachmentCreateInfo)
//            .setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });
//
//        vk::PipelineDepthStencilStateCreateInfo depthStencil = vk::PipelineDepthStencilStateCreateInfo();
//        depthStencil.depthTestEnable = VK_TRUE;
//        depthStencil.depthWriteEnable = VK_TRUE;
//        depthStencil.depthCompareOp = vk::CompareOp::eLess;
//        depthStencil.depthBoundsTestEnable = VK_FALSE;
//        depthStencil.minDepthBounds = 0.0f; // Optional
//        depthStencil.maxDepthBounds = 1.0f; // Optional
//        depthStencil.stencilTestEnable = VK_FALSE;
//        //depthStencil.front = {}; // Optional
//        //depthStencil.back = {}; // Optional
//
//        // Dynamic state
//        //vk::DynamicState dynamicStates[] = {
//        //    vk::DynamicState::eViewport,
//        //    vk::DynamicState::eLineWidth
//        //};
//        //
//        //vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
//        //    .setDynamicStateCount(2)
//        //    .setPDynamicStates(dynamicStates);
//
//        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
//            .setSetLayoutCount(static_cast<uint32_t>(m_DescriptorSetLayouts.size()))
//            .setPSetLayouts(m_DescriptorSetLayouts.data())
//            .setPushConstantRangeCount(0)
//            .setPPushConstantRanges(nullptr);
//
//        m_PipelineLayout = deviceData.LogicalDevice.createPipelineLayout(pipelineLayoutCreateInfo);
//        if (m_PipelineLayout == nullptr)
//        {
//            throw std::runtime_error("failed to create pipeline layout!");
//        }
//
//        // Create the pipeline
//        vk::GraphicsPipelineCreateInfo pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
//            .setStageCount(static_cast<uint32_t>(shaderStages.size()))
//            .setPStages(shaderStages.data())
//            .setPVertexInputState(&vertexInputStateCreateInfo)
//            .setPInputAssemblyState(&inputAssemblyStateCreateInfo)
//            .setPViewportState(&viewportStateCreateInfo)
//            .setPRasterizationState(&rasterizationStateCreateInfo)
//            .setPMultisampleState(&multisampleStateCreateInfo)
//            .setPDepthStencilState(&depthStencil)
//            .setPColorBlendState(&colorBlendStateCreateInfo)
//            .setPDynamicState(nullptr)
//            .setLayout(m_PipelineLayout)
//            .setRenderPass(m_pRenderPass->m_RenderPass)
//            .setSubpass(0)
//            .setBasePipelineHandle(VK_NULL_HANDLE)
//            .setBasePipelineIndex(-1);
//
//        if (deviceData.LogicalDevice.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_GraphicsPipeline) != vk::Result::eSuccess)
//        {
//            throw std::runtime_error("failed to create graphics pipeline!");
//        }
//	}
//
//    void DeferredPipelineTest::CreateDescriptorSets()
//    {
//        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
//        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
//        Device* pDevice = pDeviceManager->GetSelectedDevice();
//        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();
//
//        size_t imageCount = m_pRenderPass->m_Framebuffers.size();
//
//        // Create descriptor pool
//        std::array<vk::DescriptorPoolSize, 2> poolSizes{};
//        poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
//        poolSizes[0].descriptorCount = static_cast<uint32_t>(imageCount);
//
//        poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
//        poolSizes[1].descriptorCount = static_cast<uint32_t>(imageCount);
//
//        vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo();
//        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
//        poolInfo.pPoolSizes = poolSizes.data();
//        poolInfo.maxSets = static_cast<uint32_t>(imageCount);
//
//        if (deviceData.LogicalDevice.createDescriptorPool(&poolInfo, nullptr, &m_DescriptorPool) != vk::Result::eSuccess)
//            throw std::runtime_error("Failed to create descriptor pool!");
//
//        // Create uniform buffers
//        m_pUniformBufers.resize(imageCount);
//        vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
//        for (size_t i = 0; i < imageCount; i++)
//        {
//            //m_pUniformBufers[i] = new VulkanBuffer(sizeof(UniformBufferObject), (uint32_t)vk::BufferUsageFlagBits::eUniformBuffer, (uint32_t)memoryFlags);
//            //m_pUniformBufers[i] = new VulkanBuffer(sizeof(UniformBufferObject), BufferBindingTarget::B_UNIFORM, (uint32_t)memoryFlags);
//            m_pUniformBufers[i]->CreateBuffer();
//        }
//
//        // Create descriptor sets
//        std::vector<vk::DescriptorSetLayout> layouts(imageCount, m_DescriptorSetLayouts[0]);
//        vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo();
//        allocInfo.descriptorPool = m_DescriptorPool;
//        allocInfo.descriptorSetCount = static_cast<uint32_t>(imageCount);
//        allocInfo.pSetLayouts = layouts.data();
//
//        m_DescriptorSets.resize(imageCount);
//        if (deviceData.LogicalDevice.allocateDescriptorSets(&allocInfo, m_DescriptorSets.data()) != vk::Result::eSuccess)
//            throw std::runtime_error("Failed to allocate descriptor sets!");
//
//        for (size_t i = 0; i < imageCount; i++)
//        {
//            vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo();
//            bufferInfo.buffer = m_pUniformBufers[i]->GetBuffer();
//            bufferInfo.offset = 0;
//            bufferInfo.range = sizeof(UniformBufferObject);
//
//            vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo();
//            imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
//            imageInfo.imageView = m_pTexture->GetTextureImageView();
//            imageInfo.sampler = m_pTexture->GetTextureSampler();
//
//            std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};
//            descriptorWrites[0].dstSet = m_DescriptorSets[i];
//            descriptorWrites[0].dstBinding = 0;
//            descriptorWrites[0].dstArrayElement = 0;
//            descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
//            descriptorWrites[0].descriptorCount = 1;
//            descriptorWrites[0].pBufferInfo = &bufferInfo;
//            descriptorWrites[0].pImageInfo = nullptr;
//            descriptorWrites[0].pTexelBufferView = nullptr;
//
//            descriptorWrites[1].dstSet = m_DescriptorSets[i];
//            descriptorWrites[1].dstBinding = 1;
//            descriptorWrites[1].dstArrayElement = 0;
//            descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
//            descriptorWrites[1].descriptorCount = 1;
//            descriptorWrites[1].pImageInfo = &imageInfo;
//            descriptorWrites[1].pBufferInfo = nullptr;
//            descriptorWrites[1].pTexelBufferView = nullptr;
//
//            deviceData.LogicalDevice.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
//        }
//    }
//}
