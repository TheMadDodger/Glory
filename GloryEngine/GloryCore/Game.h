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

		void Initialize();
		void RunGame();
		void Destroy();
		Engine* GetEngine();

		static void Quit();

	private:
		Game(const GameSettings& pGameState);
		Game();
		virtual ~Game();

	private:
		GameState* m_pGameState;
		Engine* m_pEngine;
		bool m_bIsRunning;
		bool m_bInitialized;

		static Game m_Game;
		static bool m_bGameCreated;
	};
}