#pragma once
#include <GraphicsEnums.h>
#include <map>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Glory
{
	class SamplerSettingsComparer
	{
	public:
		bool operator()(const SamplerSettings& a, const SamplerSettings& b) const;
	};

	class VulkanImageSampler
	{
	public:
		static void Destroy();

		static VulkanImageSampler* GetSampler(const SamplerSettings& settings);
		const vk::Sampler& GetSampler() const;

	private:
		VulkanImageSampler();
		virtual ~VulkanImageSampler();

		static VulkanImageSampler* CreateNewSampler(const SamplerSettings& settings);

	private:
		static std::map<SamplerSettings, VulkanImageSampler*, SamplerSettingsComparer> m_pSamplers;
		static std::vector<VulkanImageSampler*> m_pSamplersArray;
		vk::Sampler m_TextureSampler;
	};
}
