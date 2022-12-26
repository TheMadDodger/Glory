#include "PlayerInput.h"

namespace Glory
{
	PlayerInput::PlayerInput(size_t playerIndex) : m_PlayerIndex(playerIndex), m_InputMode(InputModes::None.m_Name)
	{
	}

	PlayerInput::~PlayerInput()
	{
	}
}
