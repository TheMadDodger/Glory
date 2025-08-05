#include "DescriptorAllocator.h"
#include "VulkanDevice.h"

#include <Debug.h>

namespace Glory
{
	constexpr size_t SizeFactorCount = 11;
	constexpr std::pair<vk::DescriptorType, float> SizeFactors[SizeFactorCount] = {
		{ vk::DescriptorType::eSampler, 0.5f },
		{ vk::DescriptorType::eCombinedImageSampler, 4.f },
		{ vk::DescriptorType::eSampledImage, 4.f },
		{ vk::DescriptorType::eStorageImage, 1.f },
		{ vk::DescriptorType::eUniformTexelBuffer, 1.f },
		{ vk::DescriptorType::eStorageTexelBuffer, 1.f },
		{ vk::DescriptorType::eUniformBuffer, 2.f },
		{ vk::DescriptorType::eStorageBuffer, 2.f },
		{ vk::DescriptorType::eUniformBufferDynamic, 1.f },
		{ vk::DescriptorType::eStorageBufferDynamic, 1.f },
		{ vk::DescriptorType::eInputAttachment, 0.5f }
	};

	DescriptorAllocator::DescriptorAllocator(VulkanDevice* pDevice): m_pDevice(pDevice), m_ActivePoolIndex(0)
	{
		for (auto pool : m_DescriptorPools)
			m_pDevice->LogicalDevice().destroyDescriptorPool(pool, nullptr);
		m_DescriptorPools.clear();
	}

	DescriptorAllocator::~DescriptorAllocator()
	{
	}

	void DescriptorAllocator::ResetPools()
	{
		for (auto pool : m_DescriptorPools) {
			m_pDevice->LogicalDevice().resetDescriptorPool(pool);
		}
		m_ActivePoolIndex = 0;
	}

	void DescriptorAllocator::CreatePool(size_t count, vk::DescriptorPoolCreateFlags createFlags)
	{
		std::vector<vk::DescriptorPoolSize> poolSizes(SizeFactorCount);

		for (size_t i = 0; i < SizeFactorCount; ++i)
		{
			poolSizes[i].type = SizeFactors[i].first;
			poolSizes[i].descriptorCount = uint32_t(SizeFactors[i].second*count);
		}

		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.flags = createFlags;

		vk::DescriptorPool descriptorPool;
		if (m_pDevice->LogicalDevice().createDescriptorPool(&poolInfo, nullptr, &descriptorPool) != vk::Result::eSuccess)
		{
			m_pDevice->Debug().LogError("DescriptorAllocator::CreatePool: Failed to create desciptor pool.");
			return;
		}
		m_DescriptorPools.emplace_back(descriptorPool);
	}

	void DescriptorAllocator::Allocate(vk::DescriptorSet* set, vk::DescriptorSetLayout layout)
	{
		if (m_DescriptorPools.empty())
		{
			CreatePool(1000);
			m_ActivePoolIndex = 0;
		}

		vk::DescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.pSetLayouts = &layout;
		allocInfo.descriptorPool = m_DescriptorPools[m_ActivePoolIndex];
		allocInfo.descriptorSetCount = 1;

		//try to allocate the descriptor set
		vk::Result allocResult = m_pDevice->LogicalDevice().allocateDescriptorSets(&allocInfo, set);
		bool needReallocate = false;

		switch (allocResult) {
		case vk::Result::eSuccess:
			return;
		case vk::Result::eErrorFragmentedPool:
		case vk::Result::eErrorOutOfPoolMemory:
			break;
		default:
			return;
		}

		++m_ActivePoolIndex;
		if (m_ActivePoolIndex > m_DescriptorPools.size())
			CreatePool(1000);

		allocInfo.descriptorPool = m_DescriptorPools[m_ActivePoolIndex];
		allocResult = m_pDevice->LogicalDevice().allocateDescriptorSets(&allocInfo, set);
		if(allocResult != vk::Result::eSuccess)
			m_pDevice->Debug().LogError("DescriptorAllocator::Allocate: Failed to allocate descriptor set.");
	}
}
