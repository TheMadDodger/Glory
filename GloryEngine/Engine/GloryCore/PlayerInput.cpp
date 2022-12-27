#include "PlayerInput.h"
#include "Input.h"

namespace Glory
{
	PlayerInput::PlayerInput(InputModule* pInputModule, size_t playerIndex)
		: m_pInputModule(pInputModule), m_PlayerIndex(playerIndex), m_InputMode(InputMode::None.m_Name)
	{
	}

	PlayerInput::~PlayerInput()
	{
	}

	std::string_view PlayerInput::InputMode()
	{
		return m_InputMode;
	}
}
