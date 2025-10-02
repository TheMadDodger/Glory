#include "CommandBufferAllocator.h"
#include "VulkanDevice.h"

#include <Debug.h>

namespace Glory
{
	CommandBufferAllocator::CommandBufferAllocator(VulkanDevice* pDevice) : m_pDevice(pDevice)
	{
	}

	CommandBufferAllocator::~CommandBufferAllocator()
	{

	}

	void CommandBufferAllocator::Allocate(vk::CommandPoolCreateFlags poolflags, size_t count, vk::CommandBuffer* commandBuffers)
	{
		auto& iter = m_CommandPools.find(poolflags);
		CommandPools* commandPools = iter == m_CommandPools.end() ? CreateCommandPools(poolflags) : &iter->second;
		CommandBufferAllocator::CommandPool* pool = &commandPools->m_Pools[commandPools->m_CurrentPool];

		if (poolflags & vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		{
			/* Dynamic pool, user will manage resetting the buffers */
			const size_t firstCommandBufferIndex = pool->m_CommandBuffers.size();
			pool->m_CommandBuffers.resize(firstCommandBufferIndex + count);
			pool->m_NextCommandBufferIndex = 0;
			vk::CommandBufferAllocateInfo allocateInfo;
			allocateInfo.commandBufferCount = count;
			allocateInfo.commandPool = pool->m_VKPool;
			const vk::Result result = m_pDevice->LogicalDevice().allocateCommandBuffers(&allocateInfo, &pool->m_CommandBuffers[firstCommandBufferIndex]);
			if (result != vk::Result::eSuccess)
			{
				m_pDevice->Debug().LogError("CommandBufferAllocator::Allocate: Failed to allocate command buffers!");
				return;
			}
			std::memcpy(commandBuffers, &pool->m_CommandBuffers[firstCommandBufferIndex], count);
			return;
		}

		for (size_t i = 0; i < count; ++i)
		{
			if (pool->m_NextCommandBufferIndex >= pool->m_CommandBuffers.size())
			{
				/* Reset the last pool */
				const size_t resetIndex = commandPools->m_CurrentPool == 0 ?
					commandPools->m_Pools.size() - 1 : commandPools->m_CurrentPool - 1;
				m_pDevice->LogicalDevice().resetCommandPool(commandPools->m_Pools[resetIndex].m_VKPool);
				commandPools->m_CurrentPool = (commandPools->m_CurrentPool + 1) % commandPools->m_Pools.size();
				commandPools->m_Pools[resetIndex].m_NextCommandBufferIndex = 0;
				pool = &commandPools->m_Pools[commandPools->m_CurrentPool];
			}
			commandBuffers[i] = pool->m_CommandBuffers[pool->m_NextCommandBufferIndex];
			++pool->m_NextCommandBufferIndex;
		}
	}

	void CommandBufferAllocator::Cleanup()
	{
		for (auto& iter : m_CommandPools)
			for (auto pool : iter.second.m_Pools)
				m_pDevice->LogicalDevice().destroyCommandPool(pool.m_VKPool);
	}

	CommandBufferAllocator::CommandPools* CommandBufferAllocator::CreateCommandPools(vk::CommandPoolCreateFlags flags)
	{
		// Create command pool
		const vk::CommandPoolCreateInfo commandPoolCreateInfo = vk::CommandPoolCreateInfo()
			.setQueueFamilyIndex(m_pDevice->GraphicsFamily())
			.setFlags(flags);

		CommandPools& pools = m_CommandPools.emplace(flags, CommandPools()).first->second;
		pools.m_Pools.resize((flags & vk::CommandPoolCreateFlagBits::eResetCommandBuffer) ? 1 : 3);
		pools.m_CurrentPool = 0;

		for (size_t i = 0; i < pools.m_Pools.size(); ++i)
		{
			vk::Result result = m_pDevice->LogicalDevice().createCommandPool(&commandPoolCreateInfo, nullptr, &pools.m_Pools[i].m_VKPool);
			if (result != vk::Result::eSuccess)
			{
				m_pDevice->Debug().LogError("CommandBufferAllocator::CreateCommandPools: Failed to create command pool!");
				return &pools;
			}

			if (flags & vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
				continue;

			const size_t numBuffers = 1000;
			pools.m_Pools[i].m_CommandBuffers.resize(numBuffers);
			pools.m_Pools[i].m_NextCommandBufferIndex = 0;
			vk::CommandBufferAllocateInfo allocateInfo;
			allocateInfo.commandBufferCount = numBuffers;
			allocateInfo.commandPool = pools.m_Pools[i].m_VKPool;
			result = m_pDevice->LogicalDevice().allocateCommandBuffers(&allocateInfo, pools.m_Pools[i].m_CommandBuffers.data());
			if (result != vk::Result::eSuccess)
			{
				m_pDevice->Debug().LogError("CommandBufferAllocator::CreateCommandPools: Failed to allocate command buffers!");
				return &pools;
			}
		}
		return &pools;
	}
}