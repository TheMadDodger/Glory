#pragma once
#include "Object.h"
#include "GameSettings.h"
#include "Window.h"
#include "GameState.h"

namespace Glory
{
	class Game
	{
	public:
		static Game& CreateGame(const GameSettings &gameSettings);
		static Game& GetGame();

		void RunGame();
		void Destroy();

		static void Quit();

	private:
		Game(const GameSettings& pGameState);
		Game();
		virtual ~Game();

		void Initialize();

	private:
		GameState* m_pGameState;
		Engine* m_pEngine;
		bool m_bIsRunning;
		bool m_bInitialized;

		static Game m_Game;
		static bool m_bGameCreated;
	};
}