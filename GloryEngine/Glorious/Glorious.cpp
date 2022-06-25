// Glorious.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <Engine.h>
#include <Game.h>
#include <GameSettings.h>
#include <windows.h>
#include <GloryEngine.h>

// SHOULDNT BE HERE!
#include <SDL2/SDL_video.h>
#include <EditorCreateInfo.h>
#include <EditorApplication.h>
#include <EditorOpenGLRenderImpl.h>
#include <EditorSDLWindowImpl.h>

typedef void(__cdecl* LoadExtensionProc)(Glory::Engine*);

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    {
        //Glory::WindowCreateInfo windowCreateInfo;
        //windowCreateInfo.WindowName = "Glory Editor";
        //windowCreateInfo.Width = 2560;
        //windowCreateInfo.Height = 1300;
        //windowCreateInfo.WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

        //std::vector<Glory::Module*> optionalModules = {
        //    new Glory::SDLImageLoaderModule(),
        //    new Glory::ASSIMPModule(),
        //    new Glory::FileLoaderModule(),
        //    new Glory::MaterialLoaderModule(),
        //    new Glory::MaterialInstanceLoaderModule(),
        //    new Glory::ShaderSourceLoaderModule(),
        //};

        //std::vector<Glory::ScriptingModule*> scriptingModules = {
        //    new Glory::GloryMonoScipting(),
        //};

        //createInfo.pWindowModule = new Glory::SDLWindowModule(windowCreateInfo);
        //createInfo.pScenesModule = new Glory::EntitySceneScenesModule();
        //createInfo.pRenderModule = new Glory::ClusteredRendererModule();
        //createInfo.pGraphicsModule = new Glory::OpenGLGraphicsModule();
        //createInfo.OptionalModuleCount = static_cast<uint32_t>(optionalModules.size());
        //createInfo.pOptionalModules = optionalModules.data();
        //createInfo.ScriptingModulesCount = static_cast<uint32_t>(scriptingModules.size());
        //createInfo.pScriptingModules = scriptingModules.data();

        //Glory::EngineCreateInfo createInfo;
        //Glory::Engine* pEngine = Glory::Engine::CreateEngine(createInfo);
        //
        //HMODULE lib = LoadLibraryA("./GloryEntitiesMonoExtender.dll");
        //LoadExtensionProc loadProc = (LoadExtensionProc)GetProcAddress(lib, "LoadExtension");
        //(loadProc)(pEngine);
        //
        //Glory::GameSettings gameSettings;
        //gameSettings.pEngine = pEngine;
        //gameSettings.pGameState = new Glory::GameState();
        ////gameSettings.ApplicationType = Glory::ApplicationType::AT_Editor;
        //Glory::Game& pGame = Glory::Game::CreateGame(gameSettings);
        //pGame.Initialize();

        //std::vector<Glory::Editor::BaseEditorExtension*> editorExtensions =
        //{
        //    new Glory::Editor::EntityScenesEditorExtension(),
        //    new Glory::Editor::MonoEditorExtension()
        //};

        //Glory::EditorCreateInfo editorCreateInfo;
        //editorCreateInfo.ExtensionsCount = static_cast<uint32_t>(editorExtensions.size());
        //editorCreateInfo.pExtensions = editorExtensions.data();

        //Glory::Editor::EditorApplication editorApp(editorCreateInfo);
        //editorApp.Initialize<Glory::Editor::EditorSDLWindowImpl, Glory::Editor::EditorOpenGLRenderImpl>(pGame);
        //editorApp.Run(pGame);
        //editorApp.Destroy();

        //pGame.Destroy();
        //
        //FreeLibrary(lib);

        Glory::EngineLoader loader("./Engine.yaml");
        Glory::Engine* pEngine = loader.LoadEngine();

        Glory::GameSettings gameSettings;
        gameSettings.pEngine = pEngine;
        gameSettings.pGameState = new Glory::GameState();
        //gameSettings.ApplicationType = Glory::ApplicationType::AT_Editor;
        Glory::Game& pGame = Glory::Game::CreateGame(gameSettings);
        pGame.Initialize();

        loader.Unload();
    }

    _CrtDumpMemoryLeaks();

    return 0;
}