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

        /// Create graphics pipeline
        // Load shaders
        auto vertShaderCode = ReadFile("Shaders/vertexbuffertest_vert.spv");
        auto fragShaderCode = ReadFile("Shaders/triangle_frag.spv");

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
        const Vertex vertices[] = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
        };

        const float* verticeArray = (const float*)vertices;

        uint32_t bufferSize = sizeof(Vertex) * 4;
        vk::MemoryPropertyFlags stagingFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        VulkanBuffer* pStagingBuffer = new VulkanBuffer(bufferSize, (uint32_t)vk::BufferUsageFlagBits::eTransferSrc, (uint32_t)stagingFlags);
        pStagingBuffer->CreateBuffer();
        pStagingBuffer->Assign(verticeArray);

        vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
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
        };

        m_pMesh = new VulkanMesh(4, 6, InputRate::Vertex, 0, sizeof(Vertex), attributeTypes);
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
            .setFrontFace(vk::FrontFace::eClockwise)
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
            .setSetLayoutCount(0)
            .setPSetLayouts(nullptr)
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

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanGraphicsModule::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        if (messageSeverity != VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT && messageSeverity != VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) return VK_FALSE;
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
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
