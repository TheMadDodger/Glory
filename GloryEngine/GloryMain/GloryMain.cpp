#include "stdafx.h"
#include <SDL2/SDL_video.h>

#define _CRTDBG_MAP_ALLOC

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    {
        Glory::WindowCreateInfo windowCreateInfo;
        windowCreateInfo.WindowName = "Glory Game";
        windowCreateInfo.Width = 1280;
        windowCreateInfo.Height = 720;
        windowCreateInfo.WindowFlags = SDL_WINDOW_VULKAN;

        Glory::EngineCreateInfo createInfo;
        createInfo.pWindowModule = new Glory::SDLWindowModule(windowCreateInfo);
        createInfo.pGraphicsModule = new Glory::VulkanGraphicsModule();
        createInfo.OptionalModuleCount = 0;
        createInfo.pOptionalModules = nullptr;
        Glory::Engine* pEngine = Glory::Engine::CreateEngine(createInfo);

        Glory::GameSettings gameSettings;
        gameSettings.pEngine = pEngine;
        gameSettings.pGameState = new Glory::GameState();
        Glory::Game& pGame = Glory::Game::CreateGame(gameSettings);

        pGame.RunGame();
        pGame.Destroy();
    }

    _CrtDumpMemoryLeaks();
}
