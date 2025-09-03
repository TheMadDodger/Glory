#include "VulkanGraphicsModule.h"
#include "VulkanExceptions.h"
#include "Device.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "VulkanResourceManager.h"
#include "VulkanStructsConverter.h"

#include "VulkanDevice.h"

#include <fstream>
#include <chrono>
#include <iostream>

#include <Engine.h>
#include <Debug.h>
#include <Window.h>
#include <VertexHelpers.h>

namespace Glory
{
    GLORY_MODULE_VERSION_CPP(VulkanGraphicsModule);

    const size_t MAX_FRAMES_IN_FLIGHT = 2;

    VulkanGraphicsModule::VulkanGraphicsModule() : m_Extensions(std::vector<const char*>()), m_Layers(std::vector<const char*>()),
        m_AvailableExtensions(std::vector<VkExtensionProperties>()), m_Instance(nullptr), m_cInstance(nullptr), m_Surface(nullptr),
        m_cSurface(VK_NULL_HANDLE), m_pMainWindow(nullptr)
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

    const std::vector<const char*>& VulkanGraphicsModule::GetExtensions() const
    {
        return m_Extensions;
    }

    const std::vector<const char*>& VulkanGraphicsModule::GetValidationLayers() const
    {
        return m_Layers;
    }

    void VulkanGraphicsModule::PreInitialize()
    {
        m_pEngine->MainWindowInfo().WindowFlags |= W_Vulkan;
    }

    void VulkanGraphicsModule::OnInitialize()
    {
        /* Get the required extensions from the window */
        m_pMainWindow = m_pEngine->GetMainModule<WindowModule>()->GetMainWindow();
        m_pMainWindow->GetVulkanRequiredExtensions(m_Extensions);

        /* Use validation layers if this is a debug build */
#if defined(_DEBUG)
        InitializeValidationLayers();
#endif

        CreateVulkanInstance();
        GetAvailableExtensions();

        /* Create surface */
        m_pMainWindow->GetVulkanSurface(m_cInstance, &m_cSurface);
        m_Surface = vk::SurfaceKHR(m_cSurface);

        // Get the physical devices
        VkInstance instance = m_cInstance;
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

        std::stringstream str;
        str << "Vulkan: Available physical devices: " << deviceCount;
        m_pEngine->GetDebug().LogInfo(str.str());
        str.str("");

        if (physicalDevices.size() <= 0)
        {
            m_pEngine->GetDebug().LogFatalError("VulkanGraphicsModule::OnInitialize: No physical devices found!");
            return;
        }

        m_Devices.reserve(deviceCount);

        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        int index = 0;
        bool supportedDeviceAvailable = false;

        for (const auto& device : physicalDevices)
        {
            VulkanDevice& vkDevice = m_Devices.emplace_back(this, (vk::PhysicalDevice)device);
            vkDevice.LoadData();
            vkDevice.CheckSupport(deviceExtensions);
            supportedDeviceAvailable |= vkDevice.SupportCheckPassed();

            str << index << ": " << device;
            m_pEngine->GetDebug().LogInfo(str.str());
            str.str("");
            ++index;
            break;
        }

        if (!supportedDeviceAvailable)
        {
            m_pEngine->GetDebug().LogFatalError("VulkanGraphicsModule::OnInitialize: No supported devices found!");
            return;
        }

        for (auto& device : m_Devices)
        {
            if (!device.SupportCheckPassed()) continue;
            device.CreateLogicalDevice();
            m_pEngine->AddGraphicsDevice(&device);
        }
    }

    void VulkanGraphicsModule::OnCleanup()
    {
#if defined(_DEBUG)
        VkInstance instance = VkInstance(m_Instance);
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        VkDebugUtilsMessengerEXT debugMessenger = VkDebugUtilsMessengerEXT(m_DebugMessenger);
        if (func != nullptr) {
            func(instance, debugMessenger, nullptr);
        }
#endif
    }

    GPUResourceManager* VulkanGraphicsModule::CreateGPUResourceManager()
    {
        return new VulkanResourceManager(m_pEngine);
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
        VulkanDevice* pDevice = static_cast<VulkanDevice*>(m_pEngine->ActiveGraphicsDevice());

        vk::Sampler newSampler;
        if (pDevice->LogicalDevice().createSampler(&samplerCreateInfo, nullptr, &newSampler) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create texture sampler!");
        return m_Samplers.emplace(settings, newSampler).first->second;
    }

    void VulkanGraphicsModule::OnBeginFrame()
    {
        GraphicsModule::OnBeginFrame();
    }

    void VulkanGraphicsModule::OnEndFrame()
    {
        GraphicsModule::OnEndFrame();
    }

    vk::CommandBuffer VulkanGraphicsModule::BeginSingleTimeCommands()
    {
        VulkanDevice* pDevice = static_cast<VulkanDevice*>(m_pEngine->ActiveGraphicsDevice());
        vk::CommandPool commandPool = pDevice->GetGraphicsCommandPool();

        vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo();
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        if (pDevice->LogicalDevice().allocateCommandBuffers(&allocInfo, &commandBuffer) != vk::Result::eSuccess)
            throw std::runtime_error("Failed to allocate command buffer!");

        vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        commandBuffer.begin(beginInfo);
        return commandBuffer;
    }

    void VulkanGraphicsModule::EndSingleTimeCommands(vk::CommandBuffer commandBuffer)
    {
        VulkanDevice* pDevice = static_cast<VulkanDevice*>(m_pEngine->ActiveGraphicsDevice());
        vk::CommandPool commandPool = pDevice->GetGraphicsCommandPool();

        commandBuffer.end();

        vk::SubmitInfo submitInfo = vk::SubmitInfo();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        pDevice->GraphicsQueue().submit(1, &submitInfo, VK_NULL_HANDLE);
        pDevice->GraphicsQueue().waitIdle();
        pDevice->LogicalDevice().freeCommandBuffers(commandPool, 1, &commandBuffer);
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
