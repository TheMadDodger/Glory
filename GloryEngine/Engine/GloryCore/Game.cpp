#include "CoreExceptions.h"
#include "Game.h"
#include "GloryContext.h"
#include "Engine.h"

namespace Glory
{
	Game Game::m_Game;
	bool Game::m_bGameCreated = false;

	Game& Game::CreateGame(const GameSettings& gameSettings)
	{
		if (m_bGameCreated)
		{
			throw new GameExistsException();
		}

		m_Game = Game(gameSettings);
		gameSettings.pGameState->OnUserCreate();

		m_bGameCreated = true;
		return m_Game;
	}

	Game& Game::GetGame()
	{
		if (!m_bGameCreated)
		{
			Game* pGame = GloryContext::GetGame();
			if (pGame != nullptr)
			{
				m_Game = *pGame;
				m_bGameCreated = true;
				return m_Game;
			}
			throw new GameDoesNotExistException();
		}

		return m_Game;
	}

	void Game::RunGame()
	{
		// Initialize stuff
		Initialize();
		m_pEngine->StartThreads();

		m_Game.m_pGameState->OnUserStart();

		while (m_bIsRunning)
		{
			m_Game.m_pGameState->OnUserBeginStep();
			m_pEngine->Update();
			m_Game.m_pGameState->OnUserEndStep();

			m_Game.m_pGameState->OnUserBeginDraw();

			m_Game.m_pGameState->OnUserEndDraw();
		}

		m_pGameState->OnUserEnd();
	}

	void Game::Destroy()
	{
		if (!m_bGameCreated) return;

		if (m_pGameState)
		{
			m_pGameState->OnUserDestroy();
			delete m_pGameState;
			m_pGameState = nullptr;
		}

		delete m_pEngine;
		m_pEngine = nullptr;
		m_bGameCreated = false;
	}

	Engine* Game::GetEngine()
	{
		return m_pEngine;
	}

	void Game::Quit()
	{
		if (!m_Game.m_bInitialized) return;
		m_Game.m_bIsRunning = false;
	}

	void Game::OverrideAssetPathFunc(std::function<std::string()> func)
	{
		m_AssetPathFunc = func;
	}

	void Game::OverrideSettingsPathFunc(std::function<std::string()> func)
	{
		m_SettingsPathFunc = func;
	}

	std::string Game::GetAssetPath()
	{
		return GetGame().m_AssetPathFunc();
	}

	std::string Game::GetSettingsPath()
	{
		return GetGame().m_SettingsPathFunc();
	}

	const ApplicationType& Game::GetApplicationType() const
	{
		return m_ApplicationType;
	}

	Game::Game(const GameSettings& pGameSettings) : m_pGameState(pGameSettings.pGameState), m_pEngine(pGameSettings.pEngine),
		m_bInitialized(false), m_bIsRunning(false), m_ApplicationType(pGameSettings.ApplicationType)
	{
	}

	Game::Game() : m_pGameState(nullptr), m_pEngine(nullptr), m_bInitialized(false), m_bIsRunning(false)
	{
	}

	Game::~Game() {}

	void Game::Initialize()
	{
		if (m_bInitialized) return;
		m_AssetPathFunc = []() { return "./Assets"; };
		m_SettingsPathFunc = []() { return "./"; };

		m_pEngine->Initialize();
		m_bInitialized = true;
		m_bIsRunning = true;
	}
}
