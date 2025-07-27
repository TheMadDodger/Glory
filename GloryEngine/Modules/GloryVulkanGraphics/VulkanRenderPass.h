#pragma once
#include "SwapChain.h"
#include "DepthImage.h"

#include <Glory.h>

#include <vulkan/vulkan.hpp>

namespace Glory
{
	struct RenderPassCreateInfo
	{
		RenderPassCreateInfo();

		bool HasDepth;
		vk::Extent2D Extent;
		vk::Format Format;
		std::vector<vk::ImageView> ImageViews;
		size_t SwapChainImageCount;
	};

	class VulkanGraphicsModule;

	class VulkanRenderPass
	{
	public:
		VulkanRenderPass(VulkanGraphicsModule* pGraphics, RenderPassCreateInfo&& createInfo);
		virtual ~VulkanRenderPass();

		GLORY_API vk::RenderPass& GetRenderPass();
		GLORY_API vk::Framebuffer& GetCurrentFrameBuffer();

	private:
		void Initialize();
		void CreateSwapChainFrameBuffers();

	private:
		friend class VulkanGraphicsModule;
		friend class VulkanPipeline;
		VulkanGraphicsModule* m_pGraphics;
		vk::RenderPass m_RenderPass;
		RenderPassCreateInfo m_CreateInfo;
		std::vector<vk::Framebuffer> m_SwapChainFramebuffers;
	};
}
