#pragma once
#include <Glory.h>

#include <vulkan/vulkan.hpp>

namespace Glory
{
	class Window;
	class Device;
	struct SwapChainSupportDetails;
	class VulkanGraphicsModule;

	class SwapChain
	{
	public:
		SwapChain();
		virtual ~SwapChain();

		void Initialize(VulkanGraphicsModule* pGraphicsModule, Window* pWindow, Device* pDevice);

		GLORY_API const vk::Format& GetFormat() const;
		GLORY_API const vk::Extent2D& GetExtent() const;

		GLORY_API vk::SwapchainKHR GetSwapChain() const;
		GLORY_API size_t GetImageCount() const;
		GLORY_API vk::Image GetSwapChainImage(size_t index);
		GLORY_API vk::ImageView GetSwapChainImageView(size_t index);

	private:
		vk::SurfaceFormatKHR GetBestFormat(const SwapChainSupportDetails& details);
		vk::PresentModeKHR GetBestPresentMode(const SwapChainSupportDetails& details);
		vk::Extent2D GetSwapExtent(const SwapChainSupportDetails& details);
		uint32_t CalculateImageCount(const SwapChainSupportDetails& details);
		void CreateImageViews();

	private:
		friend class VulkanGraphicsModule;
		Window* m_pWindow;
		Device* m_pDevice;

		vk::SwapchainKHR m_SwapChain;
		std::vector<vk::Image> m_SwapChainImages;
		vk::Format m_SwapChainImageFormat;
		vk::Extent2D m_SwapChainExtent;
		std::vector<vk::ImageView> m_SwapChainImageViews;
	};
}
