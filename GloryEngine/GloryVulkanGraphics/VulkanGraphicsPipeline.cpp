#include "VulkanGraphicsPipeline.h"
#include "VulkanDeviceManager.h"
#include "Device.h"
#include <Game.h>

namespace Glory
{
	VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanRenderPass* pRenderPass, const std::vector<VulkanShader*>& pShaders, VulkanMesh* pMesh, const vk::Extent2D& swapChaindExtent)
        : m_pShaders(pShaders), m_pMesh(pMesh), m_Extent(swapChaindExtent), m_pRenderPass(pRenderPass)
	{
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        deviceData.LogicalDevice.destroyPipeline(m_GraphicsPipeline);
        deviceData.LogicalDevice.destroyPipelineLayout(m_PipelineLayout);

        for (size_t i = 0; i < m_DescriptorSetLayouts.size(); i++)
        {
            deviceData.LogicalDevice.destroyDescriptorSetLayout(m_DescriptorSetLayouts[i]);
        }

        m_DescriptorSetLayouts.clear();
	}

    void VulkanGraphicsPipeline::AddDescriptorSetLayoutInfo(const vk::DescriptorSetLayoutCreateInfo& layoutInfo)
    {
        m_DescriptorSetLayoutInfos.push_back(layoutInfo);
    }

    vk::Pipeline VulkanGraphicsPipeline::GetPipeline()
    {
        return m_GraphicsPipeline;
    }

	void VulkanGraphicsPipeline::Initialize()
	{
        VulkanGraphicsModule* pGraphics = (VulkanGraphicsModule*)Game::GetGame().GetEngine()->GetGraphicsModule();
        VulkanDeviceManager* pDeviceManager = pGraphics->GetDeviceManager();
        Device* pDevice = pDeviceManager->GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(m_pShaders.size());
        for (size_t i = 0; i < shaderStages.size(); i++)
        {
            shaderStages[i] = m_pShaders[i]->m_PipelineShaderStageInfo;
        }

        // Create descriptor set layout
        m_DescriptorSetLayouts.resize(m_DescriptorSetLayoutInfos.size());

        for (size_t i = 0; i < m_DescriptorSetLayoutInfos.size(); i++)
        {
            if (deviceData.LogicalDevice.createDescriptorSetLayout(&m_DescriptorSetLayoutInfos[0], nullptr, &m_DescriptorSetLayouts[0]) != vk::Result::eSuccess)
                throw std::runtime_error("Failed to create descriptor set layout!");
        }

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

        // Viewport and scissor
        vk::Viewport viewport = vk::Viewport()
            .setX(0.0f)
            .setY(0.0f)
            .setWidth((float)m_Extent.width)
            .setHeight((float)m_Extent.height)
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);

        vk::Rect2D scissor = vk::Rect2D()
            .setOffset({ 0,0 })
            .setExtent(m_Extent);

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

        vk::PipelineDepthStencilStateCreateInfo depthStencil = vk::PipelineDepthStencilStateCreateInfo();
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = vk::CompareOp::eLess;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        //depthStencil.front = {}; // Optional
        //depthStencil.back = {}; // Optional

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
            .setSetLayoutCount(static_cast<uint32_t>(m_DescriptorSetLayouts.size()))
            .setPSetLayouts(m_DescriptorSetLayouts.data())
            .setPushConstantRangeCount(0)
            .setPPushConstantRanges(nullptr);

        m_PipelineLayout = deviceData.LogicalDevice.createPipelineLayout(pipelineLayoutCreateInfo);
        if (m_PipelineLayout == nullptr)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        // Create the pipeline
        vk::GraphicsPipelineCreateInfo pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
            .setStageCount(static_cast<uint32_t>(shaderStages.size()))
            .setPStages(shaderStages.data())
            .setPVertexInputState(&vertexInputStateCreateInfo)
            .setPInputAssemblyState(&inputAssemblyStateCreateInfo)
            .setPViewportState(&viewportStateCreateInfo)
            .setPRasterizationState(&rasterizationStateCreateInfo)
            .setPMultisampleState(&multisampleStateCreateInfo)
            .setPDepthStencilState(&depthStencil)
            .setPColorBlendState(&colorBlendStateCreateInfo)
            .setPDynamicState(nullptr)
            .setLayout(m_PipelineLayout)
            .setRenderPass(m_pRenderPass->m_RenderPass)
            .setSubpass(0)
            .setBasePipelineHandle(VK_NULL_HANDLE)
            .setBasePipelineIndex(-1);

        if (deviceData.LogicalDevice.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_GraphicsPipeline) != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        // Cleanup
        //delete pVertShader;
        //delete pFragShader;
        //
        //delete pVertFileData;
        //delete pFragFileData;
	}
}
