#pragma once
#include <vulkan/vulkan.hpp>

#include <BitSet.h>

namespace Glory
{
	class VulkanDevice;

	class DescriptorAllocator
	{
	public:
		DescriptorAllocator(VulkanDevice* pDevice);
		virtual ~DescriptorAllocator();

		void ResetPools();

	private:
		void CreatePool(size_t count, vk::DescriptorPoolCreateFlags createFlags=vk::DescriptorPoolCreateFlags(0));
		void Allocate(vk::DescriptorSet* set, vk::DescriptorSetLayout layout);

	private:
		VulkanDevice* m_pDevice;

		std::vector<vk::DescriptorPool> m_DescriptorPools;
		size_t m_ActivePoolIndex;
	};
}
