#pragma once
#include <cstdint>
#include <Hash.h>

namespace Glory
{
	/** @brief API specific features */
	struct APIFeatures
	{
	public:
		enum Flags : uint32_t
		{
			None = 0,
			PushConstants = 1 << 0,
		};

		static constexpr Flags All = PushConstants;

	public:
		APIFeatures(Flags value) : m_Flags(value) {}
		operator Flags() const { return m_Flags; }
		bool operator==(const Flags& other) const { return m_Flags == other; }
		void operator=(const Flags& other) { m_Flags = other; }
		void operator=(const uint32_t& other) { m_Flags = Flags(other); }
		bool HasFlag(const Flags& flags) const { return (m_Flags & flags) > 0; }

	private:
		Flags m_Flags;
	};
}
