#include "PlayerInput.h"

namespace Glory
{
	PlayerInput::PlayerInput(size_t playerIndex) : m_PlayerIndex(playerIndex), m_CurrentInputMode(InputModes::None)
	{
	}

	PlayerInput::~PlayerInput()
	{
	}
}
