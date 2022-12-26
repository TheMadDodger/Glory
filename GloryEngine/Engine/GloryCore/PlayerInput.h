#pragma once
#include "Input.h"

namespace Glory
{
	struct PlayerInput
	{
	public:
		PlayerInput(size_t playerIndex);
		virtual ~PlayerInput();

	private:
		std::string_view m_InputMode;
		size_t m_PlayerIndex;
	};
}
