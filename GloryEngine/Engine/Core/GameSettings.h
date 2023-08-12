#pragma once
#include "GameState.h"

namespace Glory
{
	class Engine;

	enum class ApplicationType
	{
		AT_Game,
		AT_Editor,
	};

	struct GameSettings
	{
	public:
		GameState* pGameState;
		Engine* pEngine;
		ApplicationType ApplicationType;
	};
}
