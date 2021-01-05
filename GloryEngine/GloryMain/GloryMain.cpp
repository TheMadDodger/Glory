// GloryMain.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "stdafx.h"

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    Glory::EngineCreateInfo createInfo;
    createInfo.pWindowModule = new Glory::SDLWindowModule();
    createInfo.OptionalModuleCount = 0;
    createInfo.pOptionalModules = nullptr;
    Glory::Engine* pEngine = Glory::Engine::CreateEngine(createInfo);

    Glory::GameSettings gameSettings;
    gameSettings.pEngine = pEngine;
    gameSettings.pGameState = new Glory::GameState();
    Glory::Game& pGame = Glory::Game::CreateGame(gameSettings);

    pGame.RunGame();
    pGame.Destroy();

    _CrtDumpMemoryLeaks();
}
