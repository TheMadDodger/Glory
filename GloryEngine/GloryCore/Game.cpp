#include "CoreExceptions.h"
#include "Game.h"

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
			throw new GameDoesNotExistException();
		}

		return m_Game;
	}

	void Game::RunGame()
	{
		// Initialize stuff
		Initialize();

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

	Game::Game(const GameSettings& pGameSettings) : m_pGameState(pGameSettings.pGameState), m_pEngine(pGameSettings.pEngine),
		m_bInitialized(false), m_bIsRunning(false)
	{
	}

	Game::Game() : m_pGameState(nullptr), m_pEngine(nullptr), m_bInitialized(false), m_bIsRunning(false)
	{
	}

	Game::~Game()
	{
		Destroy();
	}

	void Game::Initialize()
	{
		if (m_bInitialized) return;
		m_pEngine->Initialize();
		m_bInitialized = true;
		m_bIsRunning = true;
	}
}
