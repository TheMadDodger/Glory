#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GraphicsModule.h>
#include <WindowModule.h>
#include "VulkanDeviceManager.h"
#include "QueueFamilyIndices.h"
#include <vulkan/vulkan.hpp>
#include "VulkanBuffer.h"
#include "VulkanMesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VulkanTexture.h"
#include "DepthImage.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphicsPipeline.h"
#include "DeferredRenderPassTest.h"
#include "DeferredPipelineTest.h"
#include "VulkanCommandBuffers.h"

namespace Glory
{
	class SwapChain;

	struct UniformBufferObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	class VulkanGraphicsModule : public GraphicsModule
	{
	public:
		VulkanGraphicsModule();
		virtual ~VulkanGraphicsModule();

		VkSurfaceKHR GetCSurface();
		vk::SurfaceKHR GetSurface();

		VkInstance GetCInstance();
		vk::Instance GetInstance();

		VulkanDeviceManager* GetDeviceManager();
		SwapChain* GetSwapChain();
		VulkanCommandBuffers* GetVulkanCommandBuffers();


		VulkanRenderPass* GetVulkanRenderPass();
		VulkanGraphicsPipeline* GetVulkanGraphicsPipeline();

		const std::vector<const char*>& GetExtensions() const;
		const std::vector<const char*>& GetValidationLayers() const;

		Buffer* CreateVertexBuffer_Internal(uint32_t bufferSize) override;

		// Temporary will need to be moved
		static vk::CommandBuffer BeginSingleTimeCommands();
		static void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);
		static void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);
		static void CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory);
		static vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);

	private:
		virtual void OnInitialize() override;
		virtual void OnCleanup() override;
		virtual FrameStates* CreateFrameStates() override;

	private:
		void InitializeValidationLayers();
		void CreateVulkanInstance();
		void GetAvailableExtensions();
		void CreateSurface();
		void LoadPhysicalDevices();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateDepthResources();
		void CreateMainRenderPass();
		void CreateDeferredRenderPassTest();
		void CreateTexture();
		void CreateMesh();
		void CreatePipeline();
		void CreateDeferredTestPipeline();
		void CreateCommandPools();
		void CreateSyncObjects();

		void UpdateUniformBuffer(uint32_t imageIndex);

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
		SwapChain* m_pSwapChain;
		DepthImage* m_pDepthImage;
		VulkanCommandBuffers* m_pCommandBuffers;
		
		
		
		
		
		
		
		VulkanRenderPass* m_pMainRenderPass;
		VulkanGraphicsPipeline* m_pGraphicsPipeline;





		DeferredRenderPassTest* m_pRenderPass;
		DeferredPipelineTest* m_pRenderPipeline;

		vk::DescriptorPool m_DescriptorPool;
		std::vector<vk::DescriptorSet> m_DescriptorSets;

		VulkanBuffer* m_pVertexBuffer;
		VulkanBuffer* m_pIndexBuffer;
		VulkanMesh* m_pMesh;
		VulkanTexture* m_pTexture;

		std::vector<VulkanBuffer*> m_pUniformBufers;

#if defined(_DEBUG)
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
#endif
	};
}