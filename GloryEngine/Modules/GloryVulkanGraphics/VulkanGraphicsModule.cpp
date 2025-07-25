#include "VulkanGraphicsModule.h"
#include "VulkanExceptions.h"
#include "Device.h"
#include "SwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "RenderPassCommandHandlers.h"
#include "PipelineCommandHandlers.h"
#include "VulkanResourceManager.h"
#include "VulkanStructsConverter.h"

#include <fstream>
#include <chrono>
#include <iostream>

#include <Engine.h>
#include <VertexHelpers.h>

namespace Glory
{
    GLORY_MODULE_VERSION_CPP(VulkanGraphicsModule);

    const size_t MAX_FRAMES_IN_FLIGHT = 2;

    VulkanGraphicsModule::VulkanGraphicsModule() : m_Extensions(std::vector<const char*>()), m_Layers(std::vector<const char*>()), m_AvailableExtensions(std::vector<VkExtensionProperties>()), m_Instance(nullptr),
        m_cInstance(nullptr), m_Surface(nullptr), m_cSurface(VK_NULL_HANDLE), m_pMainWindow(nullptr), m_CommandBuffers(this)
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

    VulkanDeviceManager& VulkanGraphicsModule::GetDeviceManager()
    {
        return m_DeviceManager;
    }

    SwapChain& VulkanGraphicsModule::GetSwapChain()
    {
        return m_SwapChain;
    }

    VulkanCommandBuffers& VulkanGraphicsModule::GetVulkanCommandBuffers()
    {
        return m_CommandBuffers;
    }

    const std::vector<const char*>& VulkanGraphicsModule::GetExtensions() const
    {
        return m_Extensions;
    }

    const std::vector<const char*>& VulkanGraphicsModule::GetValidationLayers() const
    {
        return m_Layers;
    }

    void VulkanGraphicsModule::OnInitialize()
    {
        //ShaderLoaderModule* pShaderLoader = Game::GetGame().GetEngine()->GetModule<ShaderLoaderModule>();
        //ShaderData* pShaderData = (ShaderData*)pShaderLoader->Load("./Shaders/loadertest.frag");
        //ShaderCrossCompiler compiler;
        //compiler.Compile(pShaderData->Data(), pShaderData->Size());
        //
        //// Get the required extensions from the window
        m_pMainWindow = m_pEngine->GetMainModule<WindowModule>()->GetMainWindow();
        m_pMainWindow->GetVulkanRequiredExtensions(m_Extensions);

        // Use validation layers if this is a debug build
#if defined(_DEBUG)
        InitializeValidationLayers();
#endif

        CreateVulkanInstance();
        GetAvailableExtensions();

        /* Create surface */
        m_pMainWindow->GetVulkanSurface(m_cInstance, &m_cSurface);
        m_Surface = vk::SurfaceKHR(m_cSurface);

        /* Load device */
        LoadPhysicalDevices();
        m_DeviceManager.GetSelectedDevice()->CreateLogicalDevice(this);

        /* Create swapchain and depth image */
        m_SwapChain.Initialize(this, m_pMainWindow, m_DeviceManager.GetSelectedDevice());
        m_DepthImage.Initialize(this, m_SwapChain.GetExtent());

        //CreateDepthResources();
        //CreateMainRenderPass();
        //CreateTexture();
        //CreateMesh();
        //CreatePipeline();

        m_CommandBuffers.Initialize();

        //CreateDeferredRenderPassTest();
        //CreateDeferredTestPipeline();
        //CreateCommandPools();
        //CreateSyncObjects();

        LogicalDeviceData deviceData = m_DeviceManager.GetSelectedDevice()->GetLogicalDeviceData();

        /* Create sync objects */
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_ImagesInFlight.resize(m_SwapChain.GetImageCount(), VK_NULL_HANDLE);

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

    void VulkanGraphicsModule::OnCleanup()
    {
        for (auto& iter : m_Samplers)
        {
            Device* pDevice = m_DeviceManager.GetSelectedDevice();
            LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();
            deviceData.LogicalDevice.destroySampler(iter.second, nullptr);
        }

        m_DeviceManager.GetSelectedDevice()->GetLogicalDeviceData().LogicalDevice.waitIdle();
        m_Extensions.clear();

#if defined(_DEBUG)
        VkInstance instance = VkInstance(m_Instance);
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        VkDebugUtilsMessengerEXT debugMessenger = VkDebugUtilsMessengerEXT(m_DebugMessenger);
        if (func != nullptr) {
            func(instance, debugMessenger, nullptr);
        }
#endif

        //for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        //{
        //    deviceData.LogicalDevice.destroySemaphore(m_ImageAvailableSemaphores[i]);
        //    deviceData.LogicalDevice.destroySemaphore(m_RenderFinishedSemaphores[i]);
        //    deviceData.LogicalDevice.destroyFence(m_InFlightFences[i]);
        //}
        //m_ImageAvailableSemaphores.clear();
        //m_RenderFinishedSemaphores.clear();
        //m_InFlightFences.clear();

        //delete m_pRenderPipeline;
        //m_pRenderPipeline = nullptr;

        //delete m_pRenderPass;
        //m_pRenderPass = nullptr;
    }

//    void VulkanGraphicsModule::Initialize()
//    {
//        // Get the required extensions from the window
//        m_pMainWindow = Game::GetGame().GetEngine()->GetWindowModule()->GetMainWindow();
//        m_pMainWindow->GetVulkanRequiredExtensions(m_Extensions);
//
//        // Use validation layers if this is a debug build
//#if defined(_DEBUG)
//        InitializeValidationLayers();
//#endif
//
//        CreateVulkanInstance();
//        GetAvailableExtensions();
//        CreateSurface();
//        LoadPhysicalDevices();
//        CreateLogicalDevice();
//        CreateSwapChain();
//        CreateDepthResources();
//        //CreateMainRenderPass();
//        CreateDeferredRenderPassTest();
//        CreateTexture();
//        CreateMesh();
//        //CreatePipeline();
//        CreateDeferredTestPipeline();
//
//        auto deviceData = m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
//
//        // Create descriptor pool
//        std::array<vk::DescriptorPoolSize, 2> poolSizes{};
//        poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
//        poolSizes[0].descriptorCount = static_cast<uint32_t>(m_pSwapChain->GetImageCount());
//
//        poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
//        poolSizes[1].descriptorCount = static_cast<uint32_t>(m_pSwapChain->GetImageCount());
//
//        vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo();
//        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
//        poolInfo.pPoolSizes = poolSizes.data();
//        poolInfo.maxSets = static_cast<uint32_t>(m_pSwapChain->GetImageCount());
//
//        if (deviceData.LogicalDevice.createDescriptorPool(&poolInfo, nullptr, &m_DescriptorPool) != vk::Result::eSuccess)
//            throw std::runtime_error("Failed to create descriptor pool!");
//
//        // Create uniform buffers
//        m_pUniformBufers.resize(m_pSwapChain->GetImageCount());
//        vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
//        for (size_t i = 0; i < m_pSwapChain->GetImageCount(); i++)
//        {
//            m_pUniformBufers[i] = new VulkanBuffer(sizeof(UniformBufferObject), (uint32_t)vk::BufferUsageFlagBits::eUniformBuffer, (uint32_t)memoryFlags);
//            m_pUniformBufers[i]->CreateBuffer();
//        }
//
//        // Create descriptor sets
//        std::vector<vk::DescriptorSetLayout> layouts(m_pSwapChain->GetImageCount(), m_pGraphicsPipeline->m_DescriptorSetLayouts[0]);
//        vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo();
//        allocInfo.descriptorPool = m_DescriptorPool;
//        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_pSwapChain->GetImageCount());
//        allocInfo.pSetLayouts = layouts.data();
//
//        m_DescriptorSets.resize(m_pSwapChain->GetImageCount());
//        if (deviceData.LogicalDevice.allocateDescriptorSets(&allocInfo, m_DescriptorSets.data()) != vk::Result::eSuccess)
//            throw std::runtime_error("Failed to allocate descriptor sets!");
//
//        for (size_t i = 0; i < m_pSwapChain->GetImageCount(); i++)
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
//
//        CreateCommandPools();
//        CreateSyncObjects();
//    }

//    void VulkanGraphicsModule::Cleanup()
//    {
//        m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData().LogicalDevice.waitIdle();
//        m_Extensions.clear();
//
//#if defined(_DEBUG)
//        VkInstance instance = VkInstance(m_Instance);
//        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
//        VkDebugUtilsMessengerEXT debugMessenger = VkDebugUtilsMessengerEXT(m_DebugMessenger);
//        if (func != nullptr) {
//            func(instance, debugMessenger, nullptr);
//        }
//#endif
//
//        auto deviceData = m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
//
//        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
//        {
//            deviceData.LogicalDevice.destroySemaphore(m_ImageAvailableSemaphores[i]);
//            deviceData.LogicalDevice.destroySemaphore(m_RenderFinishedSemaphores[i]);
//            deviceData.LogicalDevice.destroyFence(m_InFlightFences[i]);
//        }
//        m_ImageAvailableSemaphores.clear();
//        m_RenderFinishedSemaphores.clear();
//        m_InFlightFences.clear();
//
//        //delete m_pGraphicsPipeline;
//        //m_pGraphicsPipeline = nullptr;
//
//        delete m_pRenderPipeline;
//        m_pRenderPipeline = nullptr;
//
//        delete m_pDepthImage;
//        m_pDepthImage = nullptr;
//
//        //delete m_pMainRenderPass;
//        //m_pMainRenderPass = nullptr;
//
//        delete m_pRenderPass;
//        m_pRenderPass = nullptr;
//
//        delete m_pSwapChain;
//        m_pSwapChain = nullptr;
//
//        for (size_t i = 0; i < m_pUniformBufers.size(); i++)
//        {
//            delete m_pUniformBufers[i];
//        }
//        m_pUniformBufers.clear();
//
//        VulkanImageSampler::Destroy();
//        delete m_pTexture;
//
//        deviceData.LogicalDevice.destroyDescriptorPool(m_DescriptorPool);
//
//        delete m_pMesh;
//        m_pMesh = nullptr;
//
//        delete m_pVertexBuffer;
//        m_pVertexBuffer = nullptr;
//
//        delete m_pIndexBuffer;
//        m_pIndexBuffer = nullptr;
//
//        m_Instance.destroySurfaceKHR(m_Surface);
//        delete m_pDeviceManager;
//        m_pDeviceManager = nullptr;
//        m_Instance.destroy();
//    }

    //void VulkanGraphicsModule::OnDraw()
    //{
    //    //EnqueueCommand(TestCommand("Yo Mama"));
    //    
    //    //auto extent = m_pSwapChain->GetExtent();
    //    //BeginRenderPassCommand renderPassBegin = BeginRenderPassCommand({ 0,0 }, { extent.width, extent.height });
    //    //renderPassBegin.ClearValues.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    //    //EnqueueCommand(renderPassBegin);
    //    //EnqueueCommand(BindPipelineCommand());
    //    //EnqueueCommand(EndRenderPassCommand());
    //
    //    //auto deviceData = m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
    //    //deviceData.LogicalDevice.waitForFences(1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
    //    //
    //    //// Aqcuire swap chain image
    //    //uint32_t imageIndex;
    //    //deviceData.LogicalDevice.acquireNextImageKHR(m_pSwapChain->GetSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
    //    //
    //    //// Check if a previous frame is using this image (i.e. there is its fence to wait on)
    //    //if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
    //    //    deviceData.LogicalDevice.waitForFences(1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    //    //}
    //    //// Mark the image as now being in use by this frame
    //    //m_ImagesInFlight[imageIndex] = m_InFlightFences[m_CurrentFrame];
    //    //
    //    //UpdateUniformBuffer(imageIndex);
    //    //
    //    //// Submit command buffer
    //    //vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
    //    //vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    //    //
    //    //vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
    //    //vk::SubmitInfo submitInfo = vk::SubmitInfo()
    //    //    .setWaitSemaphoreCount(1)
    //    //    .setPWaitSemaphores(waitSemaphores)
    //    //    .setPWaitDstStageMask(waitStages)
    //    //    .setCommandBufferCount(1)
    //    //    .setPCommandBuffers(&m_CommandBuffers[imageIndex])
    //    //    .setSignalSemaphoreCount(1)
    //    //    .setPSignalSemaphores(signalSemaphores);
    //    //
    //    //deviceData.LogicalDevice.resetFences(1, &m_InFlightFences[m_CurrentFrame]);
    //    //
    //    //if (deviceData.GraphicsQueue.submit(1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != vk::Result::eSuccess)
    //    //    throw std::runtime_error("failed to submit draw command buffer!");
    //    //
    //    //vk::SwapchainKHR swapChains[] = { m_pSwapChain->GetSwapChain() };
    //    //vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
    //    //    .setWaitSemaphoreCount(1)
    //    //    .setPWaitSemaphores(signalSemaphores)
    //    //    .setSwapchainCount(1)
    //    //    .setPSwapchains(swapChains)
    //    //    .setPImageIndices(&imageIndex)
    //    //    .setPResults(nullptr);
    //    //
    //    //if (deviceData.PresentQueue.presentKHR(&presentInfo) != vk::Result::eSuccess)
    //    //    throw std::runtime_error("failed to present!");
    //    //
    //    //deviceData.PresentQueue.waitIdle();
    //    //
    //    //m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    //}

    GPUResourceManager* VulkanGraphicsModule::CreateGPUResourceManager()
    {
        return new VulkanResourceManager(m_pEngine);
    }

    //void VulkanGraphicsModule::Draw()
    //{
    //    auto deviceData = m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
    //    deviceData.LogicalDevice.waitForFences(1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

    //    // Aqcuire swap chain image
    //    uint32_t imageIndex;
    //    deviceData.LogicalDevice.acquireNextImageKHR(m_pSwapChain->GetSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

    //    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    //    if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
    //        deviceData.LogicalDevice.waitForFences(1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    //    }
    //    // Mark the image as now being in use by this frame
    //    m_ImagesInFlight[imageIndex] = m_InFlightFences[m_CurrentFrame];

    //    UpdateUniformBuffer(imageIndex);

    //    // Submit command buffer
    //    vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
    //    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    //    vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
    //    vk::SubmitInfo submitInfo = vk::SubmitInfo()
    //        .setWaitSemaphoreCount(1)
    //        .setPWaitSemaphores(waitSemaphores)
    //        .setPWaitDstStageMask(waitStages)
    //        .setCommandBufferCount(1)
    //        .setPCommandBuffers(&m_CommandBuffers[imageIndex])
    //        .setSignalSemaphoreCount(1)
    //        .setPSignalSemaphores(signalSemaphores);

    //    deviceData.LogicalDevice.resetFences(1, &m_InFlightFences[m_CurrentFrame]);

    //    if (deviceData.GraphicsQueue.submit(1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != vk::Result::eSuccess)
    //        throw std::runtime_error("failed to submit draw command buffer!");

    //    vk::SwapchainKHR swapChains[] = { m_pSwapChain->GetSwapChain() };
    //    vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
    //        .setWaitSemaphoreCount(1)
    //        .setPWaitSemaphores(signalSemaphores)
    //        .setSwapchainCount(1)
    //        .setPSwapchains(swapChains)
    //        .setPImageIndices(&imageIndex)
    //        .setPResults(nullptr);

    //    if (deviceData.PresentQueue.presentKHR(&presentInfo) != vk::Result::eSuccess)
    //        throw std::runtime_error("failed to present!");

    //    deviceData.PresentQueue.waitIdle();

    //    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    //}

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
            .setPEngineName("Glory Engine")
            .setEngineVersion(1)
            .setApiVersion(VK_API_VERSION_1_2);

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

    void VulkanGraphicsModule::LoadPhysicalDevices()
    {
        m_DeviceManager.Initialize(this);

        // Check for required device extensions
        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        m_DeviceManager.CheckDeviceSupport(this, deviceExtensions);

        // TEMPORARILY force the chosen device to the second index
        m_DeviceManager.SelectDevice(1);
    }

    //void VulkanGraphicsModule::CreateDepthResources()
    //{
    //    m_pDepthImage = new DepthImage(m_pSwapChain);
    //    m_pDepthImage->Initialize();
    //}

    //void VulkanGraphicsModule::CreateMainRenderPass()
    //{
    //    RenderPassCreateInfo createInfo{};
    //    createInfo.Extent = m_pSwapChain->GetExtent();
    //    createInfo.Format = m_pSwapChain->GetFormat();
    //    createInfo.ImageViews = m_pSwapChain->m_SwapChainImageViews;
    //    createInfo.pDepth = m_pDepthImage;
    //    createInfo.HasDepth = true;

    //    m_pMainRenderPass = new VulkanRenderPass(createInfo);
    //    m_pMainRenderPass->Initialize();
    //}

    //void VulkanGraphicsModule::CreateDeferredRenderPassTest()
    //{
    //    RenderPassCreateInfo createInfo{};
    //    createInfo.Extent = m_pSwapChain->GetExtent();
    //    createInfo.Format = m_pSwapChain->GetFormat();
    //    //createInfo.ImageViews = m_pSwapChain->m_SwapChainImageViews;
    //    createInfo.pDepth = m_pDepthImage;
    //    createInfo.HasDepth = true;
    //    createInfo.SwapChainImageCount = m_pSwapChain->GetImageCount();

    //    m_pRenderPass = new DeferredRenderPassTest(createInfo);
    //    m_pRenderPass->Initialize();
    //}

    //void VulkanGraphicsModule::CreateTexture()
    //{
    //    // Create texture
    //    ImageLoaderModule* pImageLoader = Game::GetGame().GetEngine()->GetModule<ImageLoaderModule>();
    //    ImageData* pImageData = (ImageData*)pImageLoader->Load("./Resources/viking_room_1.png");

    //    SamplerSettings samplerSettings = SamplerSettings();
    //    samplerSettings.MagFilter = Filter::F_Linear;
    //    samplerSettings.MinFilter = Filter::F_Linear;
    //    samplerSettings.AddressModeU = SamplerAddressMode::SAM_Repeat;
    //    samplerSettings.AddressModeV = SamplerAddressMode::SAM_Repeat;
    //    samplerSettings.AddressModeW = SamplerAddressMode::SAM_Repeat;
    //    samplerSettings.AnisotropyEnable = true;
    //    samplerSettings.MaxAnisotropy = m_pDeviceManager->GetSelectedDevice()->GetDeviceProperties().limits.maxSamplerAnisotropy;
    //    samplerSettings.UnnormalizedCoordinates = false;
    //    samplerSettings.CompareEnable = false;
    //    samplerSettings.CompareOp = CompareOp::OP_Always;
    //    samplerSettings.MipmapMode = Filter::F_Linear;
    //    samplerSettings.MipLODBias = 0.0f;
    //    samplerSettings.MinLOD = 0.0f;
    //    samplerSettings.MaxLOD = 0.0f;

    //    vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    //    m_pTexture = new VulkanTexture(pImageData->GetWidth(), pImageData->GetHeight(), PixelFormat::PF_RGBA, pImageData->GetFormat(), ImageType::IT_2D, (uint32_t)imageUsageFlags, (uint32_t)vk::SharingMode::eExclusive, ImageAspect::IA_Color, samplerSettings);
    //    m_pTexture->Create(pImageData);
    //}

    //void VulkanGraphicsModule::CreateMesh()
    //{
    //    // Load model
    //    ModelLoaderModule* pModelLoader = Game::GetGame().GetEngine()->GetModule<ModelLoaderModule>();
    //    ModelData* pModelData = (ModelData*)pModelLoader->Load("./Models/Cube.fbx");
    //    MeshData* pMeshData = pModelData->GetMesh(0);
    //    const float* verticeArray = (const float*)pMeshData->Vertices();

    //    uint32_t bufferSize = pMeshData->VertexSize() * pMeshData->VertexCount();
    //    vk::MemoryPropertyFlags stagingFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    //    VulkanBuffer* pStagingBuffer = nullptr;//new VulkanBuffer(bufferSize, (uint32_t)vk::BufferUsageFlagBits::eTransferSrc, (uint32_t)stagingFlags);
    //    pStagingBuffer->CreateBuffer();
    //    pStagingBuffer->Assign(verticeArray);

    //    vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
    //    vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    //    VulkanBuffer* pVertexBuffer = nullptr;//new VulkanBuffer(bufferSize, (uint32_t)usageFlags, (uint32_t)memoryFlags);
    //    pVertexBuffer->CreateBuffer();
    //    pVertexBuffer->CopyFrom(pStagingBuffer, bufferSize);
    //    m_pVertexBuffer = pVertexBuffer;
    //    delete pStagingBuffer;

    //    uint32_t indexBufferSize = sizeof(uint32_t) * pMeshData->IndexCount();
    //    pStagingBuffer = nullptr;//new VulkanBuffer(indexBufferSize, (uint32_t)vk::BufferUsageFlagBits::eTransferSrc, (uint32_t)stagingFlags);
    //    pStagingBuffer->CreateBuffer();
    //    pStagingBuffer->Assign(pMeshData->Indices());

    //    usageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    //    VulkanBuffer* pIndexBuffer = nullptr;//new VulkanBuffer(indexBufferSize, (uint32_t)usageFlags, (uint32_t)memoryFlags);
    //    pIndexBuffer->CreateBuffer();
    //    pIndexBuffer->CopyFrom(pStagingBuffer, indexBufferSize);
    //    m_pIndexBuffer = pIndexBuffer;
    //    delete pStagingBuffer;

    //    const std::vector<AttributeType> attributeTypes = {
    //        AttributeType::Float3,
    //        AttributeType::Float3,
    //        AttributeType::Float2,
    //    };

    //    m_pMesh = new VulkanMesh(pMeshData->VertexCount(), pMeshData->IndexCount(), InputRate::Vertex, 0, pMeshData->VertexSize(), attributeTypes);
    //    m_pMesh->CreateBindingAndAttributeData();
    //}

    //void VulkanGraphicsModule::CreatePipeline()
    //{
    //    /// Create graphics pipeline
    //    // Load shaders
    //    FileLoaderModule* pFileLoader = Game::GetGame().GetEngine()->GetModule<FileLoaderModule>();
    //    FileImportSettings importSettings{};
    //    importSettings.Flags = std::ios::ate | std::ios::binary;
    //    FileData* pVertFileData = (FileData*)pFileLoader->Load("./Shaders/depthbuffertest_vert.spv", importSettings);
    //    FileData* pFragFileData = (FileData*)pFileLoader->Load("./Shaders/texturetest_frag.spv", importSettings);

    //    // Create vulkan shaders
    //    VulkanShader* pVertShader = new VulkanShader(pVertFileData, ShaderType::ST_Vertex, "main");
    //    pVertShader->Initialize();
    //    VulkanShader* pFragShader = new VulkanShader(pFragFileData, ShaderType::ST_Fragment, "main");
    //    pFragShader->Initialize();

    //    std::vector<VulkanShader*> pShaders = { pVertShader, pFragShader };
    //    m_pGraphicsPipeline = new VulkanGraphicsPipeline(m_pMainRenderPass, pShaders, m_pMesh, m_pSwapChain->GetExtent());

    //    /// Create descriptor set layout
    //    vk::DescriptorSetLayoutBinding uboLayoutBinding = vk::DescriptorSetLayoutBinding();
    //    uboLayoutBinding.binding = 0;
    //    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    //    uboLayoutBinding.descriptorCount = 1;
    //    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
    //    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    //    // For the texture sampler
    //    vk::DescriptorSetLayoutBinding samplerLayoutBinding = vk::DescriptorSetLayoutBinding();
    //    samplerLayoutBinding.binding = 1;
    //    samplerLayoutBinding.descriptorCount = 1;
    //    samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    //    samplerLayoutBinding.pImmutableSamplers = nullptr;
    //    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    //    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

    //    vk::DescriptorSetLayoutCreateInfo layoutInfo = vk::DescriptorSetLayoutCreateInfo();
    //    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    //    layoutInfo.pBindings = bindings.data();

    //    m_pGraphicsPipeline->AddDescriptorSetLayoutInfo(layoutInfo);

    //    m_pGraphicsPipeline->Initialize();
    //}

    //void VulkanGraphicsModule::CreateDeferredTestPipeline()
    //{
    //    m_pRenderPipeline = new DeferredPipelineTest(m_pMesh, m_pRenderPass, m_pTexture, m_pSwapChain->GetExtent());
    //    m_pRenderPipeline->Initialize();
    //}

    //void VulkanGraphicsModule::CreatePipeline()
    //{
    //    /// Create graphics pipeline
    //    // Load shaders
    //    FileLoaderModule* pFileLoader = Game::GetGame().GetEngine()->GetModule<FileLoaderModule>();
    //    FileImportSettings importSettings{};
    //    importSettings.Flags = std::ios::ate | std::ios::binary;
    //    FileData* pVertFileData = (FileData*)pFileLoader->Load("./Shaders/depthbuffertest_vert.spv", importSettings);
    //    FileData* pFragFileData = (FileData*)pFileLoader->Load("./Shaders/texturetest_frag.spv", importSettings);
    //
    //    // Create vulkan shaders
    //    VulkanShader* pVertShader = new VulkanShader(pVertFileData, ShaderType::ST_Vertex, "main");
    //    pVertShader->Initialize();
    //    VulkanShader* pFragShader = new VulkanShader(pFragFileData, ShaderType::ST_Fragment, "main");
    //    pFragShader->Initialize();
    //
    //    auto deviceData = m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
    //
    //    vk::PipelineShaderStageCreateInfo shaderStages[] = { pVertShader->m_PipelineShaderStageInfo, pFragShader->m_PipelineShaderStageInfo };
    //
    //    // Vertex input state
    //    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
    //        .setVertexBindingDescriptionCount(1)
    //        .setPVertexBindingDescriptions(m_pMesh->GetVertexInputBindingDescription())
    //        .setVertexAttributeDescriptionCount(static_cast<uint32_t>(m_pMesh->GetVertexInputAttributeDescriptionsCount()))
    //        .setPVertexAttributeDescriptions(m_pMesh->GetVertexInputAttributeDescriptions());
    //
    //    // Input assembly
    //    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
    //        .setTopology(vk::PrimitiveTopology::eTriangleList)
    //        .setPrimitiveRestartEnable(VK_FALSE);
    //
    //    auto swapchainExtent = m_pSwapChain->GetExtent();
    //
    //    // Viewport and scissor
    //    vk::Viewport viewport = vk::Viewport()
    //        .setX(0.0f)
    //        .setY(0.0f)
    //        .setWidth((float)swapchainExtent.width)
    //        .setHeight((float)swapchainExtent.height)
    //        .setMinDepth(0.0f)
    //        .setMaxDepth(1.0f);
    //
    //    vk::Rect2D scissor = vk::Rect2D()
    //        .setOffset({ 0,0 })
    //        .setExtent(swapchainExtent);
    //
    //    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo()
    //        .setViewportCount(1)
    //        .setPViewports(&viewport)
    //        .setScissorCount(1)
    //        .setPScissors(&scissor);
    //
    //    // Rasterizer state
    //    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo()
    //        .setDepthClampEnable(VK_FALSE) // Requires a GPU feature
    //        .setRasterizerDiscardEnable(VK_FALSE)
    //        .setPolygonMode(vk::PolygonMode::eFill)
    //        .setLineWidth(1.0f)
    //        .setCullMode(vk::CullModeFlagBits::eBack)
    //        .setFrontFace(vk::FrontFace::eCounterClockwise)
    //        .setDepthBiasEnable(VK_FALSE)
    //        .setDepthBiasConstantFactor(0.0f)
    //        .setDepthBiasClamp(0.0f)
    //        .setDepthBiasSlopeFactor(0.0f);
    //
    //    // Multisampling state
    //    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo()
    //        .setSampleShadingEnable(VK_FALSE)
    //        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
    //        .setMinSampleShading(1.0f)
    //        .setPSampleMask(nullptr)
    //        .setAlphaToCoverageEnable(VK_FALSE)
    //        .setAlphaToOneEnable(VK_FALSE);
    //
    //    // Blend state
    //    vk::PipelineColorBlendAttachmentState colorBlendAttachmentCreateInfo = vk::PipelineColorBlendAttachmentState()
    //        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
    //        .setBlendEnable(VK_FALSE)
    //        .setSrcColorBlendFactor(vk::BlendFactor::eOne)
    //        .setDstColorBlendFactor(vk::BlendFactor::eZero)
    //        .setColorBlendOp(vk::BlendOp::eAdd)
    //        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
    //        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
    //        .setAlphaBlendOp(vk::BlendOp::eAdd);
    //
    //    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
    //        .setLogicOpEnable(VK_FALSE)
    //        .setLogicOp(vk::LogicOp::eCopy)
    //        .setAttachmentCount(1)
    //        .setPAttachments(&colorBlendAttachmentCreateInfo)
    //        .setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });
    //
    //    vk::PipelineDepthStencilStateCreateInfo depthStencil = vk::PipelineDepthStencilStateCreateInfo();
    //    depthStencil.depthTestEnable = VK_TRUE;
    //    depthStencil.depthWriteEnable = VK_TRUE;
    //    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    //    depthStencil.depthBoundsTestEnable = VK_FALSE;
    //    depthStencil.minDepthBounds = 0.0f; // Optional
    //    depthStencil.maxDepthBounds = 1.0f; // Optional
    //    depthStencil.stencilTestEnable = VK_FALSE;
    //    depthStencil.front = {}; // Optional
    //    depthStencil.back = {}; // Optional
    //
    //    // Dynamic state
    //    //vk::DynamicState dynamicStates[] = {
    //    //    vk::DynamicState::eViewport,
    //    //    vk::DynamicState::eLineWidth
    //    //};
    //    //
    //    //vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
    //    //    .setDynamicStateCount(2)
    //    //    .setPDynamicStates(dynamicStates);
    //
    //    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
    //        .setSetLayoutCount(1)
    //        .setPSetLayouts(&m_DescriptorSetLayout)
    //        .setPushConstantRangeCount(0)
    //        .setPPushConstantRanges(nullptr);
    //
    //    m_PipelineLayout = deviceData.LogicalDevice.createPipelineLayout(pipelineLayoutCreateInfo);
    //    if (m_PipelineLayout == nullptr)
    //    {
    //        throw std::runtime_error("failed to create pipeline layout!");
    //    }
    //
    //    // Create the pipeline
    //    vk::GraphicsPipelineCreateInfo pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
    //        .setStageCount(2)
    //        .setPStages(shaderStages)
    //        .setPVertexInputState(&vertexInputStateCreateInfo)
    //        .setPInputAssemblyState(&inputAssemblyStateCreateInfo)
    //        .setPViewportState(&viewportStateCreateInfo)
    //        .setPRasterizationState(&rasterizationStateCreateInfo)
    //        .setPMultisampleState(&multisampleStateCreateInfo)
    //        .setPDepthStencilState(&depthStencil)
    //        .setPColorBlendState(&colorBlendStateCreateInfo)
    //        .setPDynamicState(nullptr)
    //        .setLayout(m_PipelineLayout)
    //        .setRenderPass(m_pMainRenderPass->m_RenderPass)
    //        .setSubpass(0)
    //        .setBasePipelineHandle(VK_NULL_HANDLE)
    //        .setBasePipelineIndex(-1);
    //
    //    if (deviceData.LogicalDevice.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_GraphicsPipeline) != vk::Result::eSuccess)
    //    {
    //        throw std::runtime_error("failed to create graphics pipeline!");
    //    }
    //
    //    // Cleanup
    //    delete pVertShader;
    //    delete pFragShader;
    //
    //    delete pVertFileData;
    //    delete pFragFileData;
    //}

    //void VulkanGraphicsModule::CreateCommandPools()
    //{
        //auto deviceData = m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
        //vk::CommandPool commandPool = m_pDeviceManager->GetSelectedDevice()->GetGraphicsCommandPool();
        //
        //// Create command buffers
        //m_CommandBuffers.resize(m_pSwapChain->GetImageCount());
        //
        //vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
        //    .setCommandPool(commandPool)
        //    .setLevel(vk::CommandBufferLevel::ePrimary)
        //    .setCommandBufferCount((uint32_t)m_CommandBuffers.size());
        //
        //if (deviceData.LogicalDevice.allocateCommandBuffers(&commandBufferAllocateInfo, m_CommandBuffers.data()) != vk::Result::eSuccess)
        //    throw std::runtime_error("failed to allocate command buffers!");
        //
        //auto swapchainExtent = m_pSwapChain->GetExtent();
        //
        //for (size_t i = 0; i < m_CommandBuffers.size(); i++)
        //{
        //    // Start the command buffer
        //    vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo()
        //        .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
        //        .setPInheritanceInfo(nullptr);
        //
        //    if (m_CommandBuffers[i].begin(&commandBufferBeginInfo) != vk::Result::eSuccess)
        //        throw std::runtime_error("failed to begin recording command buffer!");
        //
        //    // Start a render pass
        //    vk::Rect2D renderArea = vk::Rect2D()
        //        .setOffset({ 0,0 })
        //        .setExtent(swapchainExtent);
        //
        //    vk::ClearColorValue clearColorValue = vk::ClearColorValue()
        //        .setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });
        //
        //    vk::ClearColorValue clearPosValue = vk::ClearColorValue()
        //        .setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });
        //
        //    vk::ClearColorValue clearNormalValue = vk::ClearColorValue()
        //        .setFloat32({ 0.737f, 0.737f, 1.0f, 1.0f });
        //
        //    std::array<vk::ClearValue, 4> clearColors{};
        //    clearColors[0].setColor(clearColorValue);
        //    clearColors[1].setColor(clearPosValue);
        //    clearColors[2].setColor(clearNormalValue);
        //    clearColors[3].setDepthStencil({ 1.0f, 0 });
        //
        //    vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
        //        .setRenderPass(m_pRenderPass->m_RenderPass)
        //        .setFramebuffer(m_pRenderPass->m_Framebuffers[i])
        //        .setRenderArea(renderArea)
        //        .setClearValueCount(static_cast<uint32_t>(clearColors.size()))
        //        .setPClearValues(clearColors.data());
        //
        //    m_CommandBuffers[i].beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
        //    m_CommandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pRenderPipeline->m_GraphicsPipeline);
        //
        //    vk::Buffer vertexBuffers[] = { m_pVertexBuffer->GetBuffer() };
        //    vk::DeviceSize offsets[] = { 0 };
        //    m_CommandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
        //    m_CommandBuffers[i].bindIndexBuffer(m_pIndexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
        //
        //    m_CommandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pRenderPipeline->m_PipelineLayout, 0, 1, &m_pRenderPipeline->m_DescriptorSets[i], 0, nullptr);
        //
        //    m_CommandBuffers[i].drawIndexed(static_cast<uint32_t>(m_pMesh->GetIndexCount()), 1, 0, 0, 0);
        //    m_CommandBuffers[i].endRenderPass();
        //    m_CommandBuffers[i].end();
        //}
    //}

    //void VulkanGraphicsModule::CreateCommandPools()
    //{
    //    auto deviceData = m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
    //    vk::CommandPool commandPool = m_pDeviceManager->GetSelectedDevice()->GetGraphicsCommandPool();

    //    // Create command buffers
    //    m_CommandBuffers.resize(m_pSwapChain->GetImageCount());

    //    vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
    //        .setCommandPool(commandPool)
    //        .setLevel(vk::CommandBufferLevel::ePrimary)
    //        .setCommandBufferCount((uint32_t)m_CommandBuffers.size());

    //    if (deviceData.LogicalDevice.allocateCommandBuffers(&commandBufferAllocateInfo, m_CommandBuffers.data()) != vk::Result::eSuccess)
    //        throw std::runtime_error("failed to allocate command buffers!");

    //    auto swapchainExtent = m_pSwapChain->GetExtent();

    //    for (size_t i = 0; i < m_CommandBuffers.size(); i++)
    //    {
    //        // Start the command buffer
    //        vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo()
    //            .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
    //            .setPInheritanceInfo(nullptr);

    //        if (m_CommandBuffers[i].begin(&commandBufferBeginInfo) != vk::Result::eSuccess)
    //            throw std::runtime_error("failed to begin recording command buffer!");

    //        // Start a render pass
    //        vk::Rect2D renderArea = vk::Rect2D()
    //            .setOffset({ 0,0 })
    //            .setExtent(swapchainExtent);

    //        vk::ClearColorValue clearColorValue = vk::ClearColorValue()
    //            .setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });

    //        std::array<vk::ClearValue, 2> clearColors{};
    //        clearColors[0].setColor(clearColorValue);
    //        clearColors[1].setDepthStencil({ 1.0f, 0 });

    //        vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
    //            .setRenderPass(m_pMainRenderPass->m_RenderPass)
    //            .setFramebuffer(m_pMainRenderPass->m_SwapChainFramebuffers[i])
    //            .setRenderArea(renderArea)
    //            .setClearValueCount(static_cast<uint32_t>(clearColors.size()))
    //            .setPClearValues(clearColors.data());

    //        m_CommandBuffers[i].beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
    //        m_CommandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pGraphicsPipeline->m_GraphicsPipeline);

    //        vk::Buffer vertexBuffers[] = { m_pVertexBuffer->GetBuffer() };
    //        vk::DeviceSize offsets[] = { 0 };
    //        m_CommandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
    //        m_CommandBuffers[i].bindIndexBuffer(m_pIndexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);

    //        m_CommandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pGraphicsPipeline->m_PipelineLayout, 0, 1, &m_DescriptorSets[i], 0, nullptr);

    //        m_CommandBuffers[i].drawIndexed(static_cast<uint32_t>(m_pMesh->GetIndexCount()), 1, 0, 0, 0);
    //        m_CommandBuffers[i].endRenderPass();
    //        m_CommandBuffers[i].end();
    //    }
    //}

    //void VulkanGraphicsModule::CreateSyncObjects()
    //{
        //auto deviceData = m_pDeviceManager->GetSelectedDevice()->GetLogicalDeviceData();
        //
        //// Create sync objects
        //m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        //m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        //m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        //m_ImagesInFlight.resize(m_pSwapChain->GetImageCount(), VK_NULL_HANDLE);
        //
        //vk::SemaphoreCreateInfo semaphoreCreateInfo = vk::SemaphoreCreateInfo();
        //vk::FenceCreateInfo fenceCreateInfo = vk::FenceCreateInfo()
        //    .setFlags(vk::FenceCreateFlagBits::eSignaled);
        //for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        //{
        //    if (deviceData.LogicalDevice.createSemaphore(&semaphoreCreateInfo, nullptr, &m_ImageAvailableSemaphores[i]) != vk::Result::eSuccess ||
        //        deviceData.LogicalDevice.createSemaphore(&semaphoreCreateInfo, nullptr, &m_RenderFinishedSemaphores[i]) != vk::Result::eSuccess ||
        //        deviceData.LogicalDevice.createFence(&fenceCreateInfo, nullptr, &m_InFlightFences[i]) != vk::Result::eSuccess)
        //    {
        //
        //        throw std::runtime_error("failed to create sync objects for a frame!");
        //    }
        //}
    //}

    //void VulkanGraphicsModule::UpdateUniformBuffer(uint32_t imageIndex)
    //{
    //    static auto startTime = std::chrono::high_resolution_clock::now();

    //    auto currentTime = std::chrono::high_resolution_clock::now();
    //    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    //    UniformBufferObject ubo{};
    //    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //    auto extent = m_pSwapChain->GetExtent();
    //    ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, 10.0f);
    //    ubo.proj[1][1] *= -1; // In OpenGL the Y coordinate of the clip coordinates is inverted, so we must flip it for use in Vulkan
    //    m_pRenderPipeline->m_pUniformBufers[imageIndex]->Assign(&ubo);
    //}

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanGraphicsModule::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        if (messageSeverity != VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT && messageSeverity != VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) return VK_FALSE;
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    vk::Sampler& VulkanGraphicsModule::CreateNewSampler(const SamplerSettings& settings)
    {
        // Create texture sampler
        auto samplerCreateInfo = VKConverter::GetVulkanSamplerInfo(settings);

        Device* pDevice = m_DeviceManager.GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        vk::Sampler newSampler;
        if (deviceData.LogicalDevice.createSampler(&samplerCreateInfo, nullptr, &newSampler) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create texture sampler!");
        return m_Samplers.emplace(settings, newSampler).first->second;
    }

    void VulkanGraphicsModule::OnBeginFrame()
    {
        GraphicsModule::OnBeginFrame();

        LogicalDeviceData deviceData = m_DeviceManager.GetSelectedDevice()->GetLogicalDeviceData();

        deviceData.LogicalDevice.waitForFences(1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
        deviceData.LogicalDevice.acquireNextImageKHR(m_SwapChain.GetSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_CurrentImageIndex);

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (m_ImagesInFlight[m_CurrentImageIndex] != VK_NULL_HANDLE) {
            deviceData.LogicalDevice.waitForFences(1, &m_ImagesInFlight[m_CurrentImageIndex], VK_TRUE, UINT64_MAX);
        }
        // Mark the image as now being in use by this frame
        m_ImagesInFlight[m_CurrentImageIndex] = m_InFlightFences[m_CurrentFrame];

        //UpdateUniformBuffer(imageIndex);

        // Begin the current frame command buffer
        vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
            .setPInheritanceInfo(nullptr);

        vk::CommandBuffer commandBuffer = m_CommandBuffers.GetCurrentFrameCommandBuffer();
        if (commandBuffer.begin(&commandBufferBeginInfo) != vk::Result::eSuccess)
            throw std::runtime_error("failed to begin recording command buffer!");
    }

    void VulkanGraphicsModule::OnEndFrame()
    {
        GraphicsModule::OnEndFrame();

        LogicalDeviceData deviceData = m_DeviceManager.GetSelectedDevice()->GetLogicalDeviceData();

        // End the current frame command buffer
        vk::CommandBuffer commandBuffer = m_CommandBuffers.GetCurrentFrameCommandBuffer();
        commandBuffer.end();

        // Submit command buffer
        vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
        vk::SubmitInfo submitInfo = vk::SubmitInfo()
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(waitSemaphores)
            .setPWaitDstStageMask(waitStages)
            .setCommandBufferCount(1)
            .setPCommandBuffers(&commandBuffer)
            .setSignalSemaphoreCount(1)
            .setPSignalSemaphores(signalSemaphores);

        deviceData.LogicalDevice.resetFences(1, &m_InFlightFences[m_CurrentFrame]);

        if (deviceData.GraphicsQueue.submit(1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != vk::Result::eSuccess)
            throw std::runtime_error("failed to submit draw command buffer!");

        vk::SwapchainKHR swapChains[] = { m_SwapChain.GetSwapChain() };
        vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(signalSemaphores)
            .setSwapchainCount(1)
            .setPSwapchains(swapChains)
            .setPImageIndices(&m_CurrentImageIndex)
            .setPResults(nullptr);

        if (deviceData.PresentQueue.presentKHR(&presentInfo) != vk::Result::eSuccess)
            throw std::runtime_error("failed to present!");

        deviceData.PresentQueue.waitIdle();

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    vk::CommandBuffer VulkanGraphicsModule::BeginSingleTimeCommands()
    {
        Device* pDevice = m_DeviceManager.GetSelectedDevice();
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
        Device* pDevice = m_DeviceManager.GetSelectedDevice();
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

    void VulkanGraphicsModule::TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags)
    {
        vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

        vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier();
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = aspectFlags;//vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;

        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
        {
            barrier.srcAccessMask = (vk::AccessFlags)0;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
        {
            barrier.srcAccessMask = (vk::AccessFlags)0;
            barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
        }
        else
        {
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

    void VulkanGraphicsModule::CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory)
    {
        vk::ImageCreateInfo imageInfo{};
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage = usage;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;

        Device* pDevice = m_DeviceManager.GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        if (deviceData.LogicalDevice.createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create image!");
        }

        vk::MemoryRequirements memRequirements;
        deviceData.LogicalDevice.getImageMemoryRequirements(image, &memRequirements);

        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = pDevice->GetSupportedMemoryIndex(memRequirements.memoryTypeBits, properties);

        if (deviceData.LogicalDevice.allocateMemory(&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        deviceData.LogicalDevice.bindImageMemory(image, imageMemory, 0);
    }

    vk::ImageView VulkanGraphicsModule::CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
    {
        Device* pDevice = m_DeviceManager.GetSelectedDevice();
        LogicalDeviceData deviceData = pDevice->GetLogicalDeviceData();

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = image;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vk::ImageView imageView;
        if (deviceData.LogicalDevice.createImageView(&viewInfo, nullptr, &imageView) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    const std::type_info& VulkanGraphicsModule::GetModuleType()
    {
        return typeid(VulkanGraphicsModule);
    }

    vk::Sampler& VulkanGraphicsModule::GetSampler(const SamplerSettings& settings)
    {
        auto& iter = m_Samplers.find(settings);
        if (iter != m_Samplers.end())
        {
            auto sampler = *iter;
            return sampler.second;
        }

        return CreateNewSampler(settings);
    }

    uint32_t VulkanGraphicsModule::CurrentImageIndex() const
    {
        return m_CurrentImageIndex;
    }

    void VulkanGraphicsModule::Clear(glm::vec4 color, double depth)
    {
        throw new std::exception("VulkanGraphicsModule::Clear() not yet implemented!");
    }

    void VulkanGraphicsModule::Swap()
    {
        throw new std::exception("VulkanGraphicsModule::Swap() not yet implemented!");
    }

    Material* VulkanGraphicsModule::UseMaterial(MaterialData* pMaterialData)
    {
        throw new std::exception("VulkanGraphicsModule::UseMaterial() not yet implemented!");
    }

    void VulkanGraphicsModule::OnDrawMesh(Mesh* pMesh, uint32_t vertexOffset, uint32_t vertexCount)
    {
        throw new std::exception("VulkanGraphicsModule::OnDrawMesh() not yet implemented!");
    }

    void VulkanGraphicsModule::DrawScreenQuad()
    {
        throw new std::exception("VulkanGraphicsModule::DrawScreenQuad() not yet implemented!");
    }

    void VulkanGraphicsModule::DrawUnitCube()
    {
        throw new std::exception("VulkanGraphicsModule::DrawUnitCube() not yet implemented!");
    }

    void VulkanGraphicsModule::DispatchCompute(size_t num_groups_x, size_t num_groups_y, size_t num_groups_z)
    {
        throw new std::exception("VulkanGraphicsModule::DispatchCompute() not yet implemented!");
    }

    void VulkanGraphicsModule::EnableDepthTest(bool enable)
    {
        throw new std::exception("VulkanGraphicsModule::EnableDepthTest() not yet implemented!");
    }

    void VulkanGraphicsModule::EnableDepthWrite(bool enable)
    {
        throw new std::exception("VulkanGraphicsModule::EnableDepthWrite() not yet implemented!");
    }

    void VulkanGraphicsModule::EnableStencilTest(bool enable)
    {
        throw new std::exception("VulkanGraphicsModule::EnableStencilTest() not yet implemented!");
    }

    void VulkanGraphicsModule::SetStencilMask(unsigned int mask)
    {
        throw new std::exception("VulkanGraphicsModule::SetStencilMask() not yet implemented!");
    }

    void VulkanGraphicsModule::SetStencilFunc(CompareOp func, int ref, unsigned int mask)
    {
        throw new std::exception("VulkanGraphicsModule::SetStencilFunc() not yet implemented!");
    }

    void VulkanGraphicsModule::SetStencilOP(Func fail, Func dpfail, Func dppass)
    {
        throw new std::exception("VulkanGraphicsModule::SetStencilOP() not yet implemented!");
    }

    void VulkanGraphicsModule::SetColorMask(bool r, bool g, bool b, bool a)
    {
        throw new std::exception("VulkanGraphicsModule::SetColorMask() not yet implemented!");
    }

    void VulkanGraphicsModule::ClearStencil(int value)
    {
        throw new std::exception("VulkanGraphicsModule::ClearStencil() not yet implemented!");
    }

    void VulkanGraphicsModule::SetViewport(int x, int y, uint32_t width, uint32_t height)
    {
        throw new std::exception("VulkanGraphicsModule::SetViewport() not yet implemented!");
    }

    void VulkanGraphicsModule::Scissor(int x, int y, uint32_t width, uint32_t height)
    {
        throw new std::exception("VulkanGraphicsModule::Scissor() not yet implemented!");
    }

    void VulkanGraphicsModule::EndScissor()
    {
        throw new std::exception("VulkanGraphicsModule::EndScissor() not yet implemented!");
    }

    void VulkanGraphicsModule::Blit(RenderTexture* pTexture, glm::uvec4 src, glm::uvec4 dst, Filter filter)
    {
        throw new std::exception("VulkanGraphicsModule::Blit() not yet implemented!");
    }

    void VulkanGraphicsModule::Blit(RenderTexture* pSource, RenderTexture* pDest, glm::uvec4 src, glm::uvec4 dst, Filter filter)
    {
        throw new std::exception("VulkanGraphicsModule::Blit() not yet implemented!");
    }

    void VulkanGraphicsModule::SetCullFace(CullFace cullFace)
    {
        throw new std::exception("VulkanGraphicsModule::SetCullFace() not yet implemented!");
    }

    Material* VulkanGraphicsModule::UsePassthroughMaterial()
    {
        throw new std::exception("VulkanGraphicsModule::UsePassthroughMaterial() not yet implemented!");
    }
}
