#pragma once
#include "VulkanRenderPass.h"

namespace Glory
{
	class DeferredRenderPassTest
	{
	public:
		DeferredRenderPassTest(const RenderPassCreateInfo& createInfo);
		virtual ~DeferredRenderPassTest();

	private:
		void Initialize();
		void CreateImages();
		void CreateImageViews();
		void CreateFrameBuffers();

	private:
		friend class VulkanGraphicsModule;
		friend class VulkanGraphicsPipeline;
		friend class DeferredPipelineTest;
		vk::RenderPass m_RenderPass;
		RenderPassCreateInfo m_CreateInfo;

		std::vector<vk::Image> m_PositionImages;
		std::vector<vk::DeviceMemory> m_PositionImagesMemory;
		std::vector<vk::ImageView> m_PositionImageViews;

		std::vector<vk::Image> m_ColorImages;
		std::vector<vk::DeviceMemory> m_ColorImagesMemory;
		std::vector<vk::ImageView> m_ColorImageViews;

		std::vector<vk::Image> m_NormalImages;
		std::vector<vk::DeviceMemory> m_NormalImagesMemory;
		std::vector<vk::ImageView> m_NormalImageViews;

		std::vector<vk::Framebuffer> m_Framebuffers;
	};
}
