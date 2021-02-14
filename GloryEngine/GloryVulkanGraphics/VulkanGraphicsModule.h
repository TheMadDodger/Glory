#pragma once
#define GLM_FORCE_RADIANS
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

		const std::vector<const char*>& GetExtensions() const;
		const std::vector<const char*>& GetValidationLayers() const;

		Buffer* CreateVertexBuffer_Internal(uint32_t bufferSize) override;

		// Temporary will need to be moved
		static vk::CommandBuffer BeginSingleTimeCommands();
		static void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);
		static void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

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
		std::vector<VkQueueFamilyProperties> m_AvailableQueueFamilies;
		QueueFamilyIndices m_QueueFamilyIndices;
		SwapChain* m_pSwapChain;
		vk::DescriptorPool m_DescriptorPool;
		std::vector<vk::DescriptorSet> m_DescriptorSets;

		// TEMPORARY
		vk::RenderPass m_RenderPass;
		vk::DescriptorSetLayout m_DescriptorSetLayout;
		vk::PipelineLayout m_PipelineLayout;
		vk::Pipeline m_GraphicsPipeline;
		std::vector<vk::Framebuffer> m_SwapChainFramebuffers;
		std::vector<vk::CommandBuffer> m_CommandBuffers;

		const size_t MAX_FRAMES_IN_FLIGHT = 2;
		std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
		std::vector<vk::Semaphore> m_RenderFinishedSemaphores;
		std::vector<vk::Fence> m_InFlightFences;
		std::vector<vk::Fence> m_ImagesInFlight;
		size_t m_CurrentFrame = 0;

		Buffer* m_pVertexBuffer;
		Buffer* m_pIndexBuffer;
		VulkanMesh* m_pMesh;
		VulkanTexture* m_pTexture;

		std::vector<VulkanBuffer*> m_pUniformBufers;
		
		static std::vector<char> ReadFile(const std::string& filename);

#if defined(_DEBUG)
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
#endif
	};
}