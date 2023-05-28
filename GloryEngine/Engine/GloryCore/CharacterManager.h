#pragma once
#include <cstdint>

namespace Glory
{
	class CharacterManager
	{
	public:
		uint32_t CreateCharacter();

	protected:
		virtual uint32_t CreateCharacter_Internal() = 0;
	};
}
