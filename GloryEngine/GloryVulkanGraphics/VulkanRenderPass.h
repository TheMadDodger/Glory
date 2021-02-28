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
		SwapChain* pSwapChain;
	};

	class VulkanRenderPass
	{
	public:
		VulkanRenderPass(const RenderPassCreateInfo& createInfo);
		virtual ~VulkanRenderPass();

	private:
		void Initialize();

	private:
		friend class VulkanGraphicsModule;
		vk::RenderPass m_RenderPass;
		RenderPassCreateInfo m_CreateInfo;
	};
}
