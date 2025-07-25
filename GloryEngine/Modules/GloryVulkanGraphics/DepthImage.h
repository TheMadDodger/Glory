#pragma once
#include <Glory.h>

#include <vulkan/vulkan.hpp>

namespace Glory
{
	class VulkanGraphicsModule;

	class DepthImage
	{
	public:
		DepthImage();
		virtual ~DepthImage();

		GLORY_API const vk::Format& GetFormat() const;
		GLORY_API const vk::ImageView& GetImageView() const;

	private:
		void Initialize(VulkanGraphicsModule* pGraphics, vk::Extent2D extent);

	private:
		friend class VulkanGraphicsModule;
		friend class VulkanRenderPass;
		VulkanGraphicsModule* m_pGraphics;
		vk::Format m_DepthFormat;
		vk::Image m_DepthImage;
		vk::DeviceMemory m_DepthImageMemory;
		vk::ImageView m_DepthImageView;
		vk::Extent2D m_Extent;
	};
}
