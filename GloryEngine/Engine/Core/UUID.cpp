#include "UUID.h"

#include <random>
#include <unordered_map>

namespace Glory
{
	std::random_device Device;
	std::mt19937 Random(Device());
	std::uniform_int_distribution<uint64_t> Distribution;

	UUID::UUID() : m_UUID(Distribution(Random))
	{
	}

	UUID::UUID(uint64_t uuid) : m_UUID(uuid)
	{
	}
}