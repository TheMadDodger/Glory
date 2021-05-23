#include "stdafx.h"
#include <SDL2/SDL_video.h>
#include <ASSIMPModule.h>
#include <FileLoaderModule.h>
#include <ShaderLoaderModule.h>
#include <GLSLShaderLoader.h>

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

        std::vector<Glory::Module*> optionalModules = { new Glory::SDLImageLoaderModule(), new Glory::ASSIMPModule(), new Glory::FileLoaderModule(), new Glory::GLSLShaderLoader() };

        Glory::EngineCreateInfo createInfo;
        createInfo.pWindowModule = new Glory::SDLWindowModule(windowCreateInfo);
        createInfo.pGraphicsModule = new Glory::VulkanGraphicsModule();
        createInfo.OptionalModuleCount = static_cast<uint32_t>(optionalModules.size());
        createInfo.pOptionalModules = optionalModules.data();
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
