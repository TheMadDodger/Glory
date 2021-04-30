#pragma once
#include <vulkan/vulkan.hpp>
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include "VulkanMesh.h"

namespace Glory
{
	class VulkanGraphicsPipeline
	{
	public:
		// Should use a Material object instead of a vector of shaders
		VulkanGraphicsPipeline(VulkanRenderPass* pRenderPass, const std::vector<VulkanShader*>& pShaders, VulkanMesh* pMesh, const vk::Extent2D& swapChaindExtent);
		virtual ~VulkanGraphicsPipeline();

		void AddDescriptorSetLayoutInfo(const vk::DescriptorSetLayoutCreateInfo& layoutInfo);

	private:
		void Initialize();

	private:
		friend class VulkanGraphicsModule;
		VulkanRenderPass* m_pRenderPass;
		vk::PipelineLayout m_PipelineLayout;
		vk::Pipeline m_GraphicsPipeline;

		const std::vector<VulkanShader*> m_pShaders;
		std::vector<vk::DescriptorSetLayoutCreateInfo> m_DescriptorSetLayoutInfos;
		std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
		// TEMPORARY
		// This is to get the vertex input description
		VulkanMesh* m_pMesh;
		vk::Extent2D m_Extent;
	};
}
