#include "Window.h"
#include "CoreExceptions.h"

namespace Glory
{
	Window::Window(const WindowCreateInfo& createInfo) : m_WindowName(createInfo.WindowName), m_Width(createInfo.Width), m_Height(createInfo.Height) // m_pWindow(nullptr)
	{
	}

	Window::~Window()
	{
	}

//	void Window::OpenWindow()
//	{
//        if (!SDL_Vulkan_GetInstanceExtensions(m_pWindow, &m_ExtensionCount, NULL)) {
//            std::cout << "Could not get the number of required instance extensions from SDL." << std::endl;
//        }
//        m_Extensions = std::vector<const char*>(m_ExtensionCount);
//        if (!SDL_Vulkan_GetInstanceExtensions(m_pWindow, &m_ExtensionCount, m_Extensions.data())) {
//            std::cout << "Could not get the names of required instance extensions from SDL." << std::endl;
//        }
//
//        // Use validation layers if this is a debug build
//#if defined(_DEBUG)
//        m_Layers.push_back("VK_LAYER_KHRONOS_validation");
//
//        m_Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
//        ++m_ExtensionCount;
//#endif
//
//        // Get supported validation layers
//        uint32_t layerCount;
//        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
//        std::vector<VkLayerProperties> availableLayers(layerCount);
//        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
//
//        std::cout << "Available validation layers:" << std::endl;
//
//        for (const auto& layer : availableLayers)
//        {
//            std::cout << '\t' << layer.layerName << std::endl;
//        }
//
//        for (const char* layer : m_Layers)
//        {
//            auto it = std::find_if(availableLayers.begin(), availableLayers.end(), [layer](const VkLayerProperties& otherLayer)
//            {
//                return strcmp(layer, otherLayer.layerName) == 0;
//            });
//
//            if (it == availableLayers.end()) std::cout << "The layer: " << layer << " is not available " << std::endl;
//        }
//
//        // vk::ApplicationInfo allows the programmer to specifiy some basic information about the
//        // program, which can be useful for layers and tools to provide more debug information.
//        vk::ApplicationInfo appInfo = vk::ApplicationInfo()
//            .setPApplicationName("Vulkan C++ Windowed Program Template")
//            .setApplicationVersion(1)
//            .setPEngineName("LunarG SDK")
//            .setEngineVersion(1)
//            .setApiVersion(VK_API_VERSION_1_0);
//
//        // Create debug messenger
//#if defined(_DEBUG)
//        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
//        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
//        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
//        debugCreateInfo.pfnUserCallback = DebugCallback;
//        debugCreateInfo.pUserData = nullptr; // Optional
//#endif
//
//        // vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
//        // are needed.
//        vk::InstanceCreateInfo instInfo = vk::InstanceCreateInfo()
//            .setFlags(vk::InstanceCreateFlags())
//            .setPApplicationInfo(&appInfo)
//            .setEnabledExtensionCount(static_cast<uint32_t>(m_Extensions.size()))
//            .setPpEnabledExtensionNames(m_Extensions.data())
//            .setEnabledLayerCount(static_cast<uint32_t>(m_Layers.size()))
//#if defined(_DEBUG)
//            .setPNext(&debugCreateInfo)
//#endif
//            .setPpEnabledLayerNames(m_Layers.data());
//
//        // Create the Vulkan instance.
//        try {
//            m_Instance = vk::createInstance(instInfo);
//        }
//        catch (const std::exception & e) {
//            std::cout << "Could not create a Vulkan instance: " << e.what() << std::endl;
//        }
//
//        VkInstance instance = VkInstance(m_Instance);
//
//        // Create debug messenger
//#if defined(_DEBUG)
//        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
//        if (func != nullptr) {
//            VkDebugUtilsMessengerEXT debugMessenger;
//            func(instance, &debugCreateInfo, nullptr, &debugMessenger);
//            m_DebugMessenger = vk::DebugUtilsMessengerEXT(debugMessenger);
//        }
//        else {
//            std::cout << "Could not find the create debug messenger function." << std::endl;
//        }
//#endif
//
//        // Check extension support
//        uint32_t extensionCount = 0;
//        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
//        std::vector<VkExtensionProperties> extensions(extensionCount);
//        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
//
//        std::cout << "Available extensions:" << std::endl;
//        for (const auto& extension : extensions)
//        {
//            std::cout << '\t' << extension.extensionName << std::endl;
//        }
//
//        // Create a Vulkan surface for rendering
//        if (!SDL_Vulkan_CreateSurface(m_pWindow, static_cast<VkInstance>(m_Instance), &m_cSurface)) {
//            std::cout << "Could not create a Vulkan surface." << std::endl;
//        }
//        m_Surface = vk::SurfaceKHR(m_cSurface);
//
//        // Get the physical devices
//        uint32_t deviceCount = 0;
//        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
//        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
//        vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
//        std::cout << "Available physical devices:" << deviceCount << std::endl;
//        for (const auto& device : physicalDevices)
//        {
//            std::cout << '\t' << device << std::endl;
//        }
//
//        if (physicalDevices.size() <= 0)
//        {
//            std::cerr << "No physical device found!" << std::endl;
//            return;
//        }
//
//        auto chosenDevice = physicalDevices[1];
//        m_PhysicalDevice = vk::PhysicalDevice(chosenDevice);
//
//        // Find queue families
//        uint32_t queueFamilyCount = 0;
//        vkGetPhysicalDeviceQueueFamilyProperties(chosenDevice, &queueFamilyCount, nullptr);
//        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
//        vkGetPhysicalDeviceQueueFamilyProperties(chosenDevice, &queueFamilyCount, queueFamilies.data());
//
//        int i = 0;
//        for (const auto& queueFamily : queueFamilies)
//        {
//            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
//            {
//                graphicsFamily = i;
//            }
//            VkBool32 presentSupport = false;
//            vkGetPhysicalDeviceSurfaceSupportKHR(chosenDevice, i, m_cSurface, &presentSupport);
//            if (presentSupport) presentFamily = i;
//
//            i++;
//        }
//        
//        if (!graphicsFamily.has_value())
//        {
//            std::cerr << "Graphics queue family not found!" << std::endl;
//            return;
//        }
//
//        if (!presentFamily.has_value())
//        {
//            std::cerr << "Present queue family not found!" << std::endl;
//            return;
//        }
//        
//        std::cout << "Graphics queue family found!" << std::endl;
//
//        // Check for required device extensions
//        const std::vector<const char*> deviceExtensions = {
//            VK_KHR_SWAPCHAIN_EXTENSION_NAME
//        };
//
//        uint32_t deviceExtensionCount;
//        vkEnumerateDeviceExtensionProperties(chosenDevice, nullptr, &deviceExtensionCount, nullptr);
//        std::vector<VkExtensionProperties> availableExtensions(deviceExtensionCount);
//        vkEnumerateDeviceExtensionProperties(chosenDevice, nullptr, &deviceExtensionCount, availableExtensions.data());
//
//        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
//
//        for (const auto& extension : availableExtensions)
//        {
//            requiredExtensions.erase(extension.extensionName);
//        }
//
//        if (!requiredExtensions.empty())
//        {
//            std::cerr << "Missing required device extensions!" << std::endl;
//            return;
//        }
//
//        // Check swapchain support
//        SwapChainSupportDetails details;
//        m_PhysicalDevice.getSurfaceCapabilitiesKHR(m_Surface, &details.Capabilities);
//        uint32_t formatCount = 0;
//        details.Formats = m_PhysicalDevice.getSurfaceFormatsKHR(m_Surface);
//        details.PresentModes = m_PhysicalDevice.getSurfacePresentModesKHR(m_Surface);
//
//        bool swapChainAdequite = !details.Formats.empty() && !details.PresentModes.empty();
//        if (!swapChainAdequite)
//        {
//            std::cerr << "Swapchain not supported!" << std::endl;
//            return;
//        }
//
//        // Create logical device
//        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
//        std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily.value(), presentFamily.value() };
//
//        float queuePriority = 1.0f;
//        for (uint32_t queueFamily : uniqueQueueFamilies) {
//            vk::DeviceQueueCreateInfo queueCreateInfo = vk::DeviceQueueCreateInfo()
//                .setQueueFamilyIndex(queueFamily)
//                .setQueueCount(1)
//                .setPQueuePriorities(&queuePriority);
//            queueCreateInfos.push_back(queueCreateInfo);
//        }
//
//        vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();
//        vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo()
//            .setPQueueCreateInfos(queueCreateInfos.data())
//            .setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
//            .setPEnabledFeatures(&deviceFeatures)
//            .setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()))
//            .setPpEnabledExtensionNames(deviceExtensions.data());
//
//#if defined(_DEBUG)
//        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_Layers.size());
//        deviceCreateInfo.ppEnabledLayerNames = m_Layers.data();
//#else
//        deviceCreateInfo.enabledLayerCount = 0;
//#endif // _DEBUG
//
//        m_Device = m_PhysicalDevice.createDevice(deviceCreateInfo, nullptr);
//
//        // Get the queue family from the device
//        m_GraphicsQueue = m_Device.getQueue(graphicsFamily.value(), 0);
//        m_PresentQueue = m_Device.getQueue(presentFamily.value(), 0);
//
//        // Choose best format
//        vk::SurfaceFormatKHR chosenFormat = details.Formats[0];
//        for (const auto& availableFormat : details.Formats)
//        {
//            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
//            {
//                chosenFormat = availableFormat;
//                break;
//            }
//        }
//
//        // Choose best presentation mode
//        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
//        for (const auto& availablePresentMode : details.PresentModes)
//        {
//            if (availablePresentMode == vk::PresentModeKHR::eMailbox)
//            {
//                presentMode = vk::PresentModeKHR::eMailbox;
//                break;
//            }
//        }
//
//        // Choose best swap extent
//        vk::Extent2D swapExtent;
//
//        if (details.Capabilities.currentExtent.width != UINT32_MAX) {
//            swapExtent = details.Capabilities.currentExtent;
//        }
//        else
//        {
//            int width, height;
//            SDL_GL_GetDrawableSize(m_pWindow, &width, &height);
//
//            swapExtent = {
//                static_cast<uint32_t>(width),
//                static_cast<uint32_t>(height)
//            };
//
//            // Clamp width and height between the allowed minimum and maximum sizes
//            swapExtent.width = std::max(details.Capabilities.minImageExtent.width, std::min(details.Capabilities.maxImageExtent.width, swapExtent.width));
//            swapExtent.height = std::max(details.Capabilities.minImageExtent.height, std::min(details.Capabilities.maxImageExtent.height, swapExtent.height));
//        }
//
//        uint32_t imageCount = details.Capabilities.minImageCount + 1;
//        if (details.Capabilities.minImageCount > 0 && imageCount > details.Capabilities.minImageCount)
//            imageCount = details.Capabilities.minImageCount;
//
//        vk::SwapchainCreateInfoKHR swapChainCreateInfo = vk::SwapchainCreateInfoKHR()
//            .setSurface(m_Surface)
//            .setMinImageCount(imageCount)
//            .setImageFormat(chosenFormat.format)
//            .setImageColorSpace(chosenFormat.colorSpace)
//            .setImageExtent(swapExtent)
//            .setImageArrayLayers(1)
//            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment); // vk::ImageUsageFlagBits::eTransferDst is used when copying an image to the swap chain, which is required when doing post processing!
//
//        uint32_t queueFamilyIndices[] = { graphicsFamily.value(), presentFamily.value() };
//
//        if (graphicsFamily != presentFamily) {
//            swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
//            swapChainCreateInfo.queueFamilyIndexCount = 2;
//            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
//        }
//        else {
//            swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
//            swapChainCreateInfo.queueFamilyIndexCount = 0; // Optional
//            swapChainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
//        }
//
//        swapChainCreateInfo.preTransform = details.Capabilities.currentTransform;
//        swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
//        swapChainCreateInfo.presentMode = presentMode;
//        swapChainCreateInfo.clipped = VK_TRUE;
//        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
//        m_SwapChain = m_Device.createSwapchainKHR(swapChainCreateInfo, nullptr);
//        if (m_SwapChain == nullptr)
//        {
//            std::cerr << "Could not create swapchain!" << std::endl;
//            return;
//        }
//
//        uint32_t swapChainImageCount = 0;
//        m_SwapChainImages = m_Device.getSwapchainImagesKHR(m_SwapChain);
//
//        m_SwapChainImageFormat = chosenFormat.format;
//        m_SwapChainExtent = swapExtent;
//
//        // Create image views
//        m_SwapChainImageViews.resize(m_SwapChainImages.size());
//
//        for (size_t i = 0; i < m_SwapChainImages.size(); i++)
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
//                .setImage(m_SwapChainImages[i])
//                .setViewType(vk::ImageViewType::e2D)
//                .setFormat(m_SwapChainImageFormat)
//                .setComponents(componentMapping)
//                .setSubresourceRange(subResourceRange);
//
//            m_SwapChainImageViews[i] = m_Device.createImageView(imageViewCreateInfo, nullptr);
//
//            if (m_SwapChainImageViews[i] == nullptr)
//            {
//                std::cerr << "Could not create image view!" << std::endl;
//                return;
//            }
//        }
//
//        // Create render pass
//        vk::AttachmentDescription colorAttachment = vk::AttachmentDescription()
//            .setFormat(m_SwapChainImageFormat)
//            .setSamples(vk::SampleCountFlagBits::e1)
//            .setLoadOp(vk::AttachmentLoadOp::eClear)
//            .setStoreOp(vk::AttachmentStoreOp::eStore)
//            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
//            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
//            .setInitialLayout(vk::ImageLayout::eUndefined)
//            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
//
//        vk::AttachmentReference colorAttachmentRef = vk::AttachmentReference()
//            .setAttachment(0)
//            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
//
//        vk::SubpassDescription subPass = vk::SubpassDescription()
//            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
//            .setColorAttachmentCount(1)
//            .setPColorAttachments(&colorAttachmentRef);
//
//        vk::SubpassDependency dependancy = vk::SubpassDependency()
//            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
//            .setDstSubpass(0)
//            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
//            .setSrcAccessMask((vk::AccessFlags)0)
//            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
//            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
//
//        vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo()
//            .setAttachmentCount(1)
//            .setPAttachments(&colorAttachment)
//            .setSubpassCount(1)
//            .setPSubpasses(&subPass)
//            .setDependencyCount(1)
//            .setPDependencies(&dependancy);
//
//        m_RenderPass = m_Device.createRenderPass(renderPassCreateInfo);
//        if (m_RenderPass == nullptr)
//        {
//            throw std::runtime_error("failed to create render pass!");
//        }
//
//        /// Create graphics pipeline
//        // Load shaders
//        auto vertShaderCode = ReadFile("Shaders/triangle_vert.spv");
//        auto fragShaderCode = ReadFile("Shaders/triangle_frag.spv");
//
//        // Create shader modules
//        vk::ShaderModule vertShaderModule;
//        vk::ShaderModule fragShaderModule;
//
//        vk::ShaderModuleCreateInfo shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
//            .setCodeSize(vertShaderCode.size())
//            .setPCode(reinterpret_cast<const uint32_t*>(vertShaderCode.data()));
//        vertShaderModule = m_Device.createShaderModule(shaderModuleCreateInfo, nullptr);
//
//        shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
//            .setCodeSize(fragShaderCode.size())
//            .setPCode(reinterpret_cast<const uint32_t*>(fragShaderCode.data()));
//        fragShaderModule = m_Device.createShaderModule(shaderModuleCreateInfo, nullptr);
//
//        vk::PipelineShaderStageCreateInfo vertShaderStageInfo = vk::PipelineShaderStageCreateInfo()
//            .setStage(vk::ShaderStageFlagBits::eVertex)
//            .setModule(vertShaderModule)
//            .setPName("main");
//
//        vk::PipelineShaderStageCreateInfo fragShaderStageInfo = vk::PipelineShaderStageCreateInfo()
//            .setStage(vk::ShaderStageFlagBits::eFragment)
//            .setModule(fragShaderModule)
//            .setPName("main");
//
//        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo , fragShaderStageInfo };
//
//        // Vertex input state
//        vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
//            .setVertexBindingDescriptionCount(0)
//            .setPVertexAttributeDescriptions(nullptr)
//            .setVertexAttributeDescriptionCount(0)
//            .setPVertexAttributeDescriptions(nullptr);
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
//            .setWidth((float)m_SwapChainExtent.width)
//            .setHeight((float)m_SwapChainExtent.height)
//            .setMinDepth(0.0f)
//            .setMaxDepth(1.0f);
//
//        vk::Rect2D scissor = vk::Rect2D()
//            .setOffset({ 0,0 })
//            .setExtent(m_SwapChainExtent);
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
//            .setFrontFace(vk::FrontFace::eClockwise)
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
//        vk::PipelineColorBlendAttachmentState colorBlendAttachmentCreateInfo = vk::PipelineColorBlendAttachmentState()
//            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
//            .setBlendEnable(VK_FALSE)
//            .setSrcColorBlendFactor(vk::BlendFactor::eOne)
//            .setDstColorBlendFactor(vk::BlendFactor::eZero)
//            .setColorBlendOp(vk::BlendOp::eAdd)
//            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
//            .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
//            .setAlphaBlendOp(vk::BlendOp::eAdd);
//
//        vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
//            .setLogicOpEnable(VK_FALSE)
//            .setLogicOp(vk::LogicOp::eCopy)
//            .setAttachmentCount(1)
//            .setPAttachments(&colorBlendAttachmentCreateInfo)
//            .setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });
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
//            .setSetLayoutCount(0)
//            .setPSetLayouts(nullptr)
//            .setPushConstantRangeCount(0)
//            .setPPushConstantRanges(nullptr);
//
//        m_PipelineLayout = m_Device.createPipelineLayout(pipelineLayoutCreateInfo);
//        if (m_PipelineLayout == nullptr)
//        {
//            throw std::runtime_error("failed to create pipeline layout!");
//        }
//
//        // Create the pipeline
//        vk::GraphicsPipelineCreateInfo pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
//            .setStageCount(2)
//            .setPStages(shaderStages)
//            .setPVertexInputState(&vertexInputStateCreateInfo)
//            .setPInputAssemblyState(&inputAssemblyStateCreateInfo)
//            .setPViewportState(&viewportStateCreateInfo)
//            .setPRasterizationState(&rasterizationStateCreateInfo)
//            .setPMultisampleState(&multisampleStateCreateInfo)
//            .setPDepthStencilState(nullptr)
//            .setPColorBlendState(&colorBlendStateCreateInfo)
//            .setPDynamicState(nullptr)
//            .setLayout(m_PipelineLayout)
//            .setRenderPass(m_RenderPass)
//            .setSubpass(0)
//            .setBasePipelineHandle(VK_NULL_HANDLE)
//            .setBasePipelineIndex(-1);
//
//        if (m_Device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_GraphicsPipeline) != vk::Result::eSuccess)
//        {
//            throw std::runtime_error("failed to create graphics pipeline!");
//        }
//
//        m_Device.destroyShaderModule(vertShaderModule);
//        m_Device.destroyShaderModule(fragShaderModule);
//
//        // Create framebuffers
//        m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());
//        for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
//        {
//            vk::ImageView attachments[] = {
//                m_SwapChainImageViews[i]
//            };
//
//            vk::FramebufferCreateInfo frameBufferCreateInfo = vk::FramebufferCreateInfo()
//                .setRenderPass(m_RenderPass)
//                .setAttachmentCount(1)
//                .setPAttachments(attachments)
//                .setWidth(m_SwapChainExtent.width)
//                .setHeight(m_SwapChainExtent.height)
//                .setLayers(1);
//
//            m_SwapChainFramebuffers[i] = m_Device.createFramebuffer(frameBufferCreateInfo);
//            if(m_SwapChainFramebuffers[i] == nullptr)
//                throw std::runtime_error("failed to create framebuffer!");
//        }
//
//        // Create command pool
//        vk::CommandPoolCreateInfo commandPoolCreateInfo = vk::CommandPoolCreateInfo()
//            .setQueueFamilyIndex(graphicsFamily.value())
//            .setFlags((vk::CommandPoolCreateFlags)0);
//
//        m_CommandPool = m_Device.createCommandPool(commandPoolCreateInfo);
//        if (m_CommandPool == nullptr)
//            throw std::runtime_error("failed to create command pool!");
//
//        // Create command buffers
//        m_CommandBuffers.resize(m_SwapChainFramebuffers.size());
//
//        vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
//            .setCommandPool(m_CommandPool)
//            .setLevel(vk::CommandBufferLevel::ePrimary)
//            .setCommandBufferCount((uint32_t)m_CommandBuffers.size());
//
//        if(m_Device.allocateCommandBuffers(&commandBufferAllocateInfo, m_CommandBuffers.data()) != vk::Result::eSuccess)
//            throw std::runtime_error("failed to allocate command buffers!");
//
//        for (size_t i = 0; i < m_CommandBuffers.size(); i++)
//        {
//            // Start the command buffer
//            vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo()
//                .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
//                .setPInheritanceInfo(nullptr);
//
//            if(m_CommandBuffers[i].begin(&commandBufferBeginInfo) != vk::Result::eSuccess)
//                throw std::runtime_error("failed to begin recording command buffer!");
//
//            // Start a render pass
//            vk::Rect2D renderArea = vk::Rect2D()
//                .setOffset({ 0,0 })
//                .setExtent(m_SwapChainExtent);
//
//            vk::ClearColorValue clearColorValue = vk::ClearColorValue()
//                .setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });
//
//            vk::ClearValue clearColor = vk::ClearValue()
//                .setColor(clearColorValue);
//
//            vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
//                .setRenderPass(m_RenderPass)
//                .setFramebuffer(m_SwapChainFramebuffers[i])
//                .setRenderArea(renderArea)
//                .setClearValueCount(1)
//                .setPClearValues(&clearColor);
//
//            m_CommandBuffers[i].beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
//            m_CommandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline);
//            m_CommandBuffers[i].draw(3, 1, 0, 0);
//            m_CommandBuffers[i].endRenderPass();
//            m_CommandBuffers[i].end();
//        }
//
//        // Create semaphores
//        vk::SemaphoreCreateInfo semaphoreCreateInfo = vk::SemaphoreCreateInfo();
//        if(m_Device.createSemaphore(&semaphoreCreateInfo, nullptr, &m_ImageAvailableSemaphore) != vk::Result::eSuccess || m_Device.createSemaphore(&semaphoreCreateInfo, nullptr, &m_RenderFinishedSemaphore) != vk::Result::eSuccess)
//            throw std::runtime_error("failed to create semaphores!");
//	}
//
//    void Window::CloseWindow()
//    {
//        m_Device.waitIdle();
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
//        m_Device.destroySemaphore(m_ImageAvailableSemaphore);
//        m_Device.destroySemaphore(m_RenderFinishedSemaphore);
//
//        m_Device.destroyPipeline(m_GraphicsPipeline);
//        m_Device.destroyPipelineLayout(m_PipelineLayout);
//
//        for (size_t i = 0; i < m_SwapChainFramebuffers.size(); i++)
//        {
//            m_Device.destroyFramebuffer(m_SwapChainFramebuffers[i]);
//        }
//        m_SwapChainFramebuffers.clear();
//        
//        m_Device.destroyRenderPass(m_RenderPass);
//        m_Device.destroyCommandPool(m_CommandPool);
//        m_Device.destroySwapchainKHR(m_SwapChain);
//        m_Instance.destroySurfaceKHR(m_Surface);
//        for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
//        {
//            m_Device.destroyImageView(m_SwapChainImageViews[i]);
//        }
//        m_SwapChainImageViews.clear();
//        m_SwapChainImages.clear();
//
//        m_Device.destroy();
//        m_Instance.destroy();
//        SDL_DestroyWindow(m_pWindow);
//    }
//
//    void Window::DrawWindow()
//    {
//        // Aqcuire swap chain image
//        uint32_t imageIndex;
//        m_Device.acquireNextImageKHR(m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
//
//        // Submit command buffer
//        vk::Semaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
//        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
//
//        vk::Semaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
//        vk::SubmitInfo submitInfo = vk::SubmitInfo()
//            .setWaitSemaphoreCount(1)
//            .setPWaitSemaphores(waitSemaphores)
//            .setPWaitDstStageMask(waitStages)
//            .setCommandBufferCount(1)
//            .setPCommandBuffers(&m_CommandBuffers[imageIndex])
//            .setSignalSemaphoreCount(1)
//            .setPSignalSemaphores(signalSemaphores);
//
//        if(m_GraphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE) != vk::Result::eSuccess)
//            throw std::runtime_error("failed to submit draw command buffer!");
//
//        vk::SwapchainKHR swapChains[] = { m_SwapChain };
//        vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
//            .setWaitSemaphoreCount(1)
//            .setPWaitSemaphores(signalSemaphores)
//            .setSwapchainCount(1)
//            .setPSwapchains(swapChains)
//            .setPImageIndices(&imageIndex)
//            .setPResults(nullptr);
//
//        if(m_PresentQueue.presentKHR(&presentInfo) != vk::Result::eSuccess)
//            throw std::runtime_error("failed to present!");
//    }
//
//    VKAPI_ATTR VkBool32 VKAPI_CALL Window::DebugCallback(
//        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
//        VkDebugUtilsMessageTypeFlagsEXT messageType,
//        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
//        void* pUserData)
//    {
//        if (messageSeverity != VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT && messageSeverity != VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) return VK_FALSE;
//
//        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
//
//        return VK_FALSE;
//    }
//
//    std::vector<char> Window::ReadFile(const std::string& filename)
//    {
//        std::ifstream file(filename, std::ios::ate | std::ios::binary);
//
//        if (!file.is_open())
//        {
//            throw std::runtime_error("failed to open file!");
//        }
//
//        size_t fileSize = (size_t)file.tellg();
//        std::vector<char> buffer(fileSize);
//        file.seekg(0);
//        file.read(buffer.data(), fileSize);
//        file.close();
//        return buffer;
//    }
}