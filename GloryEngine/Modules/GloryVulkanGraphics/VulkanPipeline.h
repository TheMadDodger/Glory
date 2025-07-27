#pragma once
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include "VulkanMesh.h"

#include <Pipeline.h>
#include <vulkan/vulkan.hpp>

namespace Glory
{
	class PipelineData;

	class VulkanPipeline : public Pipeline
	{
	public:
		// Should use a Material object instead of a vector of shaders
		VulkanPipeline(PipelineData* pPipeline, VulkanRenderPass* pRenderPass, VulkanMesh* pMesh, const vk::Extent2D& swapChaindExtent);
		virtual ~VulkanPipeline();

		void AddDescriptorSetLayoutInfo(const vk::DescriptorSetLayoutCreateInfo& layoutInfo);

		vk::Pipeline GetPipeline();

	private:
		void Initialize();

	private:
		friend class VulkanGraphicsModule;
		VulkanRenderPass* m_pRenderPass;
		vk::PipelineLayout m_PipelineLayout;
		vk::Pipeline m_GraphicsPipeline;

		std::vector<vk::DescriptorSetLayoutCreateInfo> m_DescriptorSetLayoutInfos;
		std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
		// TEMPORARY
		// This is to get the vertex input description
		VulkanMesh* m_pMesh;
		vk::Extent2D m_Extent;
	};
}
