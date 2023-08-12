#pragma once
#include <vulkan/vulkan.hpp>

namespace Glory
{
	class VulkanGraphicsModule;

	class VulkanCommandBuffers
	{
	public:
		VulkanCommandBuffers(VulkanGraphicsModule* pModule);
		virtual ~VulkanCommandBuffers();

		void Initialize();

		vk::CommandBuffer GetCurrentFrameCommandBuffer();

	private:
		VulkanGraphicsModule* m_pModule;
		std::vector<vk::CommandBuffer> m_FrameCommandBuffers;

		vk::CommandPoolCreateFlags m_FrameCommandPoolFlags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	};
}
