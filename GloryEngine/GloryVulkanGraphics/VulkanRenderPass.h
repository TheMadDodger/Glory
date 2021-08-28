#pragma once
#include <vulkan/vulkan.hpp>
#include "SwapChain.h"
#include "DepthImage.h"

namespace Glory
{
	struct AttachmentInfo
	{
		AttachmentInfo();

		bool Present;

	};

	struct RenderPassCreateInfo
	{
		RenderPassCreateInfo();

		bool HasDepth;
		DepthImage* pDepth;
		vk::Extent2D Extent;
		vk::Format Format;
		std::vector<vk::ImageView> ImageViews;
		size_t SwapChainImageCount;
	};

	class VulkanRenderPass
	{
	public:
		VulkanRenderPass(const RenderPassCreateInfo& createInfo);
		virtual ~VulkanRenderPass();

		vk::RenderPass GetRenderPass();
		vk::Framebuffer GetCurrentFrameBuffer();

	private:
		void Initialize();
		void CreateSwapChainFrameBuffers();

	private:
		friend class VulkanGraphicsModule;
		friend class VulkanGraphicsPipeline;
		vk::RenderPass m_RenderPass;
		RenderPassCreateInfo m_CreateInfo;
		std::vector<vk::Framebuffer> m_SwapChainFramebuffers;
	};
}
