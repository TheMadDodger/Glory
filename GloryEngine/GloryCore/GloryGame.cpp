#include "stdafx.h"
#include "GloryGame.h"
#include "GloryCore.h"

namespace Glory
{
	GloryGame GloryGame::m_Game;
	bool GloryGame::m_bGameCreated = false;

	GloryGame& GloryGame::CreateGame(const GloryGameSettings& gameSettings)
	{
		if (m_bGameCreated)
		{
			throw new GameExistsException();
		}

		m_Game = GloryGame(gameSettings);
		gameSettings.pGameState->OnUserCreate();

		m_bGameCreated = true;
		return m_Game;
	}

	GloryGame& GloryGame::GetGame()
	{
		if (!m_bGameCreated)
		{
			throw new GameDoesNotExistException();
		}

		return m_Game;
	}

	void GloryGame::RunGame()
	{
		// Initialize stuff
		Initialize();

		m_Game.m_pGameState->OnUserStart();

		while (m_bIsRunning)
		{
			m_Game.m_pGameState->OnUserBeginStep();
			SDL_Event event;
			while (SDL_PollEvent(&event)) {

				switch (event.type) {

				case SDL_QUIT:
					m_bIsRunning = false;
					break;

				default:
					// Do nothing.
					break;
				}
			}

			// Logic stuff
			m_Game.m_pGameState->OnUserEndStep();

			m_Game.m_pGameState->OnUserBeginDraw();
			// Drawing stuff
			m_Game.m_pGameState->OnUserEndDraw();
		}

		m_pGameState->OnUserEnd();

		SDL_Delay(10);
	}

	GloryGame::GloryGame(const GloryGameSettings& pGameSettings) : m_pGameState(pGameSettings.pGameState), m_pWindow(nullptr), m_bInitialized(false)
	{
	}

	GloryGame::GloryGame() : m_pGameState(nullptr), m_pWindow(nullptr), m_bInitialized(false)
	{
	}

	GloryGame::~GloryGame()
	{
		if (!m_bGameCreated) return;

		if (m_pGameState)
		{
			m_pGameState->OnUserDestroy();
			delete m_pGameState;
			m_pGameState = nullptr;
		}

		m_bGameCreated = false;

		delete m_pWindow;
		GloryCore::Destroy();
	}

	void GloryGame::Initialize()
	{
		if (m_bInitialized) return;

		GloryCore::Initialize();
		m_pWindow = new Window("Glory Game");
		m_pWindow->OpenWindow();

		m_bInitialized = true;
	}
}