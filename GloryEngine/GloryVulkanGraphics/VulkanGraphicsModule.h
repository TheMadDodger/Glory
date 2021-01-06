#pragma once
#include <GraphicsModule.h>
#include <WindowModule.h>
#include "VulkanDeviceManager.h"
#include "QueueFamilyIndices.h"
#include <vulkan/vulkan.hpp>

namespace Glory
{
	class SwapChain;

	class VulkanGraphicsModule : public GraphicsModule
	{
	public:
		VulkanGraphicsModule();
		virtual ~VulkanGraphicsModule();

		VkSurfaceKHR GetCSurface();
		vk::SurfaceKHR GetSurface();

		VkInstance GetCInstance();
		vk::Instance GetInstance();

		const std::vector<const char*>& GetExtensions() const;
		const std::vector<const char*>& GetValidationLayers() const;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
		virtual void Update() override;
		virtual void Draw() override;

	private:
		void InitializeValidationLayers();
		void CreateVulkanInstance();
		void GetAvailableExtensions();
		void CreateSurface();
		void LoadPhysicalDevices();
		void CreateLogicalDevice();
		void CreateSwapChain();

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	private:
		std::vector<const char*> m_Extensions;
		std::vector<const char*> m_Layers;
		std::vector<VkExtensionProperties> m_AvailableExtensions;
		vk::Instance m_Instance;
		VkInstance m_cInstance;
		vk::SurfaceKHR m_Surface;
		VkSurfaceKHR m_cSurface;
		Window* m_pMainWindow;
		VulkanDeviceManager* m_pDeviceManager;
		std::vector<VkQueueFamilyProperties> m_AvailableQueueFamilies;
		QueueFamilyIndices m_QueueFamilyIndices;
		SwapChain* m_pSwapChain;

		// TEMPORARY
		vk::RenderPass m_RenderPass;
		vk::PipelineLayout m_PipelineLayout;
		vk::Pipeline m_GraphicsPipeline;
		std::vector<vk::Framebuffer> m_SwapChainFramebuffers;
		vk::CommandPool m_CommandPool;
		std::vector<vk::CommandBuffer> m_CommandBuffers;
		vk::Semaphore m_ImageAvailableSemaphore;
		vk::Semaphore m_RenderFinishedSemaphore;
		
		static std::vector<char> ReadFile(const std::string& filename);

#if defined(_DEBUG)
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
#endif
	};
}