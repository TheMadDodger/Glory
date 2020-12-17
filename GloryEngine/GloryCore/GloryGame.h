#pragma once
#include "Object.h"
#include "GloryGameSettings.h"
#include "Window.h"

namespace Glory
{
	class GloryGame
	{
	public:
		static GloryGame& CreateGame(const GloryGameSettings &pGameState);
		static GloryGame& GetGame();

		void RunGame();

	private:
		GloryGame(const GloryGameSettings& pGameState);
		GloryGame();
		virtual ~GloryGame();

		void Initialize();

	private:
		GloryGameState* m_pGameState;
		Window* m_pWindow;
		bool m_bIsRunning;
		bool m_bInitialized = false;

		static GloryGame m_Game;
		static bool m_bGameCreated;
	};
}