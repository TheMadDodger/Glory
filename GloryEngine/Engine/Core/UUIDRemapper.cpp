#include "UUIDRemapper.h"
#include "DistributedRandom.h"

#include <random>

namespace Glory
{
	UUIDRemapper::UUIDRemapper():
		m_pRandom(nullptr),
		m_IsPassThrough(true),
		m_Seed(0)
	{
	}

	UUIDRemapper::UUIDRemapper(uint32_t seed):
		m_pRandom(seed != 0 ? new DistributedRandom<uint64_t>(seed) : nullptr),
		m_IsPassThrough(seed == 0),
		m_Seed(seed)
	{
	}

	UUIDRemapper::~UUIDRemapper()
	{
		if (m_pRandom == nullptr) return;
		delete m_pRandom;
		m_pRandom = nullptr;
	}

	UUID UUIDRemapper::operator()(const UUID uuid)
	{
		if (uuid == 0) return 0;
		if (m_IsPassThrough) return uuid;
		auto itor = m_Results.find(uuid);
		if (itor != m_Results.end()) return itor->second;

		const uint32_t first32Bits = uint32_t((uuid << 32) >> 32);
		const uint32_t second32Bits = uint32_t(uuid >> 32);
		const uint32_t otherSeed = first32Bits & second32Bits;
		const uint32_t seed = m_Seed & otherSeed;

		m_pRandom->Reset(seed);
		const UUID result = m_pRandom->Next();
		m_Results.emplace(uuid, result);
		return result;
	}

	bool UUIDRemapper::Find(const UUID uuid, UUID& outUUID) const
	{
		if (m_IsPassThrough)
		{
			outUUID = uuid;
			return true;
		}
		auto itor = m_Results.find(uuid);
		if (itor == m_Results.end()) return false;
		outUUID = itor->second;
		return true;
	}

	void UUIDRemapper::SoftReset(uint32_t seed)
	{
		m_Seed = seed;
		if (m_pRandom == nullptr)
		{
			m_pRandom = new DistributedRandom<uint64_t>(seed);
			m_IsPassThrough = false;
			return;
		}

		m_pRandom->Reset(seed);
	}

	void UUIDRemapper::SoftReset()
	{
		SoftReset(RandomDevice::Seed());
	}

	void UUIDRemapper::Reset()
	{
		Reset(RandomDevice::Seed());
	}

	void UUIDRemapper::Reset(uint32_t seed)
	{
		m_Results.clear();
		SoftReset(seed);
	}

	void UUIDRemapper::EnforceRemap(UUID one, UUID two)
	{
		m_Results.emplace(one, two);
	}
}
