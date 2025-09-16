#include "VulkanGraphicsModule.h"
#include "VulkanExceptions.h"
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

    void VulkanGraphicsModule::Initialize()
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

    void VulkanGraphicsModule::Cleanup()
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
}
