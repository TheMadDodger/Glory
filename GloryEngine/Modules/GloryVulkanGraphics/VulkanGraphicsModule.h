#pragma once
#include <GraphicsModule.h>
#include <WindowModule.h>
#include <GraphicsEnums.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.hpp>

namespace Glory
{
	class VulkanDevice;

	class VulkanGraphicsModule : public GraphicsModule
	{
	public:
		VulkanGraphicsModule();
		virtual ~VulkanGraphicsModule();

		GLORY_API VkSurfaceKHR GetCSurface();
		GLORY_API vk::SurfaceKHR GetSurface();

		GLORY_API VkInstance GetCInstance();
		GLORY_API vk::Instance GetInstance();

		GLORY_API const std::vector<const char*>& GetExtensions() const;
		GLORY_API const std::vector<const char*>& GetValidationLayers() const;

		/** @brief VulkanGraphicsModule type */
		const std::type_info& GetModuleType() override;

		GLORY_API vk::Sampler& GetSampler(const SamplerSettings& settings);

		GLORY_MODULE_VERSION_H(0, 1, 0);

	private:
		virtual void PreInitialize() override;
		virtual void Initialize() override;
		virtual void Cleanup() override;

	private:
		void InitializeValidationLayers();
		void CreateVulkanInstance();
		void GetAvailableExtensions();

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
		
		std::map<SamplerSettings, vk::Sampler, SamplerSettingsComparer> m_Samplers;

		std::vector<VulkanDevice> m_Devices;

#if defined(_DEBUG)
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
#endif
	};
}