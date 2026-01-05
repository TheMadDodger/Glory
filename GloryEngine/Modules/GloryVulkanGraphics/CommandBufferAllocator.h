#pragma once
#include <vector>
#include <map>

#include <vulkan/vulkan.hpp>

namespace Glory
{
	class VulkanDevice;


	class CommandBufferAllocator
	{
	public:
		CommandBufferAllocator(VulkanDevice* pDevice);
		virtual ~CommandBufferAllocator();

		void Allocate(vk::CommandPoolCreateFlags poolflags, size_t count, vk::CommandBuffer* commandBuffers);
		void Cleanup();

	private:

		struct CommandPool
		{
			vk::CommandPool m_VKPool;
			size_t m_NextCommandBufferIndex = 0;
			std::vector<vk::CommandBuffer> m_CommandBuffers;
		};

		struct CommandPools
		{
			size_t m_CurrentPool = 0;
			std::vector<CommandPool> m_Pools;
		};
		CommandPools* CreateCommandPools(vk::CommandPoolCreateFlags flags);

	private:
		VulkanDevice* m_pDevice;
		std::map<vk::CommandPoolCreateFlags, CommandPools> m_CommandPools;
	};
}