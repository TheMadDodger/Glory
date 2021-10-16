// GloryEditor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//#include <imgui.h>
//#include <Device.h>
//#include "EditorSDLWindowImpl.h"
//#include "EditorVulkanRenderImpl.h"
//#include "EditorApplication.h"
//
//#define _CRTDBG_MAP_ALLOC
//
//using namespace Glory::Editor;
//
//int main()
//{
//    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//
//    {
//        Glory::WindowCreateInfo windowCreateInfo;
//        windowCreateInfo.WindowName = "Glory Editor";
//        windowCreateInfo.Width = 2560;
//        windowCreateInfo.Height = 1300;
//        windowCreateInfo.WindowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
//
//        std::vector<Glory::Module*> optionalModules = { new Glory::SDLImageLoaderModule(), new Glory::ASSIMPModule(), new Glory::FileLoaderModule(), new Glory::GLSLShaderLoader() };
//
//        Glory::EngineCreateInfo createInfo;
//        createInfo.pWindowModule = new Glory::SDLWindowModule(windowCreateInfo);
//
//        Glory::VulkanGraphicsModule* pGraphicsModule = new Glory::VulkanGraphicsModule();
//
//        createInfo.pGraphicsModule = pGraphicsModule;
//        createInfo.OptionalModuleCount = static_cast<uint32_t>(optionalModules.size());
//        createInfo.pOptionalModules = optionalModules.data();
//        Glory::Engine* pEngine = Glory::Engine::CreateEngine(createInfo);
//
//        Glory::GameSettings gameSettings;
//        gameSettings.pEngine = pEngine;
//        gameSettings.pGameState = new Glory::GameState();
//        Glory::Game& pGame = Glory::Game::CreateGame(gameSettings);
//        pGame.Initialize();
//
//        EditorApplication editorApp;
//        editorApp.Initialize<EditorSDLWindowImpl, EditorVulkanRenderImpl>();
//        editorApp.Run();
//        editorApp.Destroy();
//
//        pGame.Destroy();
//    }
//
//    _CrtDumpMemoryLeaks();
//
//    return 0;
//}
//