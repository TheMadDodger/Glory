#pragma once
#include <cstdint>
#include <string>
#include <Reflection.h>

namespace Glory
{
	class Engine;

	struct LayerMask
	{
		LayerMask() : m_Mask(0) {}
		LayerMask(uint64_t mask) : m_Mask(mask) {}

		operator uint64_t() const { return m_Mask; }
		LayerMask& operator |=(const LayerMask& other);
		LayerMask& operator ^=(const LayerMask& other);
		LayerMask operator &(const LayerMask& other) const;

		static LayerMask FromString(Engine* pEngine, const std::string& names);

		REFLECTABLE(LayerMask, (uint64_t)(m_Mask))
	};
}
