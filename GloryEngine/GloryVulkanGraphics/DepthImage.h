#pragma once
#include <vulkan/vulkan.hpp>
#include "SwapChain.h"

namespace Glory
{
	class DepthImage
	{
	public:
		DepthImage(SwapChain* pSwapChain);
		virtual ~DepthImage();

	private:
		void Initialize();

	private:
		friend class VulkanGraphicsModule;
		friend class VulkanRenderPass;
		SwapChain* m_pSwapChain;
		vk::Format m_DepthFormat;
		vk::Image m_DepthImage;
		vk::DeviceMemory m_DepthImageMemory;
		vk::ImageView m_DepthImageView;
	};
}
