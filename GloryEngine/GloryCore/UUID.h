#pragma once
#include <cstdint>
#include <xhash>

namespace Glory
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		~UUID();

		operator uint64_t() const { return m_UUID; }

	private:
		uint64_t m_UUID;
	};
}

namespace std
{
	template<>
	struct hash<Glory::UUID>
	{
		std::size_t operator()(const Glory::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}
