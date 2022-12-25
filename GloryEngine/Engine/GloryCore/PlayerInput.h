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
		InputMode m_CurrentInputMode;
		size_t m_PlayerIndex;
	};
}
