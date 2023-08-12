#include "UUID.h"
#include <random>
#include <unordered_map>

namespace Glory
{
	static std::random_device s_Device;
	static std::mt19937 s_Random(s_Device());
	static std::uniform_int_distribution<uint64_t> s_Distribution;

	UUID::UUID() : m_UUID(s_Distribution(s_Random))
	{
	}

	UUID::UUID(uint64_t uuid) : m_UUID(uuid)
	{
	}

	UUID::~UUID()
	{
	}
}