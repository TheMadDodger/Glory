#pragma once
#include <cstdint>
#include <string>

namespace Glory
{
	struct LayerMask
	{
		LayerMask() : m_Mask(0) {}
		LayerMask(uint64_t mask) : m_Mask(mask) {}

		operator uint64_t() const { return m_Mask; }
		LayerMask& operator |=(const LayerMask& other);
		LayerMask& operator ^=(const LayerMask& other);
		LayerMask operator &(const LayerMask& other);
		uint64_t m_Mask;

		static LayerMask FromString(const std::string& names);
	};
}
