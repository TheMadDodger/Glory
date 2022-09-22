#pragma once
#include "Object.h"
#include "GameSettings.h"
#include "Window.h"
#include "Glory.h"

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

		void OverrideAssetPathFunc(std::function<std::string()> func);
		static std::string GetAssetPath();

		const ApplicationType& GetApplicationType() const;

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
		std::function<std::string()> m_AssetPathFunc;
		ApplicationType m_ApplicationType;
	};
}