#pragma once
#include <vulkan/vulkan.hpp>
#include "DeferredRenderPassTest.h"
#include "VulkanShader.h"
#include "VulkanMesh.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"

namespace Glory
{
	class DeferredPipelineTest
	{
	public:
		DeferredPipelineTest(VulkanMesh* pMesh, DeferredRenderPassTest* pRenderPass, VulkanTexture* pTexture, const vk::Extent2D& extent);
		virtual ~DeferredPipelineTest();

	private:
		void Initialize();
		void CreateDescriptorSets();

	private:
		friend class VulkanGraphicsModule;
		DeferredRenderPassTest* m_pRenderPass;
		vk::PipelineLayout m_PipelineLayout;
		vk::Pipeline m_GraphicsPipeline;

		std::vector<VulkanShader*> m_pShaders;
		std::vector<vk::DescriptorSetLayoutCreateInfo> m_DescriptorSetLayoutInfos;
		std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
		// TEMPORARY
		// This is to get the vertex input description
		VulkanMesh* m_pMesh;
		VulkanTexture* m_pTexture;
		vk::Extent2D m_Extent;

		vk::DescriptorPool m_DescriptorPool;
		std::vector<vk::DescriptorSet> m_DescriptorSets;
		std::vector<VulkanBuffer*> m_pUniformBufers;
	};
}
