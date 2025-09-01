#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "VulkanDeviceManager.h"
#include "QueueFamilyIndices.h"
#include "VulkanBuffer.h"
#include "VulkanMesh.h"
#include "VulkanTexture.h"
#include "DepthImage.h"
#include "VulkanRenderPass.h"
#include "DeferredRenderPassTest.h"
#include "DeferredPipelineTest.h"
#include "VulkanCommandBuffers.h"

#include <GraphicsModule.h>
#include <WindowModule.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.hpp>

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

		GLORY_API VkSurfaceKHR GetCSurface();
		GLORY_API vk::SurfaceKHR GetSurface();

		GLORY_API VkInstance GetCInstance();
		GLORY_API vk::Instance GetInstance();

		GLORY_API VulkanDeviceManager& GetDeviceManager();
		GLORY_API SwapChain& GetSwapChain();
		GLORY_API DepthImage& GetDepthImage();
		GLORY_API VulkanCommandBuffers& GetVulkanCommandBuffers();

		GLORY_API const std::vector<const char*>& GetExtensions() const;
		GLORY_API const std::vector<const char*>& GetValidationLayers() const;

		// Temporary will need to be moved
		GLORY_API vk::CommandBuffer BeginSingleTimeCommands();
		GLORY_API void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);
		GLORY_API void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);
		GLORY_API void CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory);
		GLORY_API vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);

		/** @brief VulkanGraphicsModule type */
		const std::type_info& GetModuleType() override;

		GLORY_API vk::Sampler& GetSampler(const SamplerSettings& settings);

		GLORY_API uint32_t CurrentImageIndex() const;
		GLORY_API uint32_t ImageCount() const;

		GLORY_API VulkanRenderPass& MainRenderPass() const;

		GLORY_MODULE_VERSION_H(0, 1, 0);

	public: // Commands
		virtual void Clear(glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), double depth = 1.0) override;
		virtual void Swap() override;
		virtual Material* UseMaterial(MaterialData* pMaterialData) override;
		virtual void OnDrawMesh(Mesh* pMesh, uint32_t vertexOffset, uint32_t vertexCount) override;
		virtual void DrawScreenQuad() override;
		virtual void DrawUnitCube() override;
		virtual void DispatchCompute(size_t num_groups_x, size_t num_groups_y, size_t num_groups_z) override;
		virtual void EnableDepthTest(bool enable) override;
		virtual void EnableDepthWrite(bool enable) override;
		virtual void EnableStencilTest(bool enable) override;
		virtual void SetStencilMask(unsigned int mask) override;
		virtual void SetStencilFunc(CompareOp func, int ref, unsigned int mask) override;
		virtual void SetStencilOP(Func fail, Func dpfail, Func dppass) override;
		virtual void SetColorMask(bool r, bool g, bool b, bool a) override;
		virtual void ClearStencil(int value) override;
		virtual void SetViewport(int x, int y, uint32_t width, uint32_t height) override;
		virtual void Scissor(int x, int y, uint32_t width, uint32_t height) override;
		virtual void EndScissor() override;
		virtual void Blit(RenderTexture* pTexture, glm::uvec4 src = glm::uvec4(), glm::uvec4 dst = glm::uvec4(),
			Filter filter = Filter::F_Nearest) override;
		virtual void Blit(RenderTexture* pSource, RenderTexture* pDest, glm::uvec4 src = glm::uvec4(), glm::uvec4 dst = glm::uvec4(),
			Filter filter = Filter::F_Nearest) override;
		virtual void SetCullFace(CullFace cullFace) override;

		virtual Material* UsePassthroughMaterial() override;

	private:
		virtual void PreInitialize() override;
		virtual void OnInitialize() override;
		virtual void OnCleanup() override;
		virtual GPUResourceManager* CreateGPUResourceManager() override;

	private:
		void InitializeValidationLayers();
		void CreateVulkanInstance();
		void GetAvailableExtensions();
		void LoadPhysicalDevices();

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

		vk::Sampler& CreateNewSampler(const SamplerSettings& settings);

		virtual void OnBeginFrame() override;
		virtual void OnEndFrame() override;

	private:
		std::vector<const char*> m_Extensions;
		std::vector<const char*> m_Layers;
		std::vector<VkExtensionProperties> m_AvailableExtensions;

		vk::Instance m_Instance;
		VkInstance m_cInstance;
		vk::SurfaceKHR m_Surface;
		VkSurfaceKHR m_cSurface;
		Window* m_pMainWindow;

		VulkanDeviceManager m_DeviceManager;
		SwapChain m_SwapChain;
		DepthImage m_DepthImage;
		VulkanCommandBuffers m_CommandBuffers;
		std::unique_ptr<VulkanRenderPass> m_MainRenderPass;
		
		std::map<SamplerSettings, vk::Sampler, SamplerSettingsComparer> m_Samplers;
		
		uint32_t m_CurrentFrame = 0;
		uint32_t m_CurrentImageIndex = 0;

		std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
		std::vector<vk::Semaphore> m_RenderFinishedSemaphores;
		std::vector<vk::Fence> m_InFlightFences;
		std::vector<vk::Fence> m_ImagesInFlight;
		

		//VulkanRenderPass* m_pMainRenderPass;
		//VulkanGraphicsPipeline* m_pGraphicsPipeline;

		//DeferredRenderPassTest* m_pRenderPass;
		//DeferredPipelineTest* m_pRenderPipeline;

		//vk::DescriptorPool m_DescriptorPool;
		//std::vector<vk::DescriptorSet> m_DescriptorSets;

		//VulkanBuffer* m_pVertexBuffer;
		//VulkanBuffer* m_pIndexBuffer;
		//VulkanMesh* m_pMesh;
		//VulkanTexture* m_pTexture;

		//std::vector<VulkanBuffer*> m_pUniformBufers;

#if defined(_DEBUG)
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
#endif
	};
}