#pragma once
#include "GameState.h"
#include "Engine.h"

namespace Glory
{
	struct GameSettings
	{
	public:
		GameState* pGameState;
		Engine* pEngine;
	};
}
