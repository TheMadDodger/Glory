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
        Glory::WindowCreateInfo windowCreateInfo;
        windowCreateInfo.WindowName = "Glory Editor";
        windowCreateInfo.Width = 2560;
        windowCreateInfo.Height = 1300;
        //windowCreateInfo.WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
        windowCreateInfo.WindowFlags = 2 | 32;

        //std::vector<Glory::ScriptingModule*> scriptingModules = {
        //    new Glory::GloryMonoScipting(),
        //};
        // 
        //createInfo.ScriptingModulesCount = static_cast<uint32_t>(scriptingModules.size());
        //createInfo.pScriptingModules = scriptingModules.data();

        Glory::EngineLoader loader("./Engine.yaml");
        Glory::Engine* pEngine = loader.LoadEngine(windowCreateInfo);

        Glory::GameSettings gameSettings;
        gameSettings.pEngine = pEngine;
        gameSettings.pGameState = new Glory::GameState();
        //gameSettings.ApplicationType = Glory::ApplicationType::AT_Editor;
        Glory::Game& pGame = Glory::Game::CreateGame(gameSettings);
        pGame.Initialize();

        //std::vector<Glory::Editor::BaseEditorExtension*> editorExtensions =
        //{
        //    new Glory::Editor::EntityScenesEditorExtension(),
        //    new Glory::Editor::MonoEditorExtension()
        //};

        Glory::EditorCreateInfo editorCreateInfo{};
        //editorCreateInfo.ExtensionsCount = static_cast<uint32_t>(editorExtensions.size());
        //editorCreateInfo.pExtensions = editorExtensions.data();

        //Glory::Editor::EditorApplication editorApp(editorCreateInfo);
        //editorApp.Initialize<Glory::Editor::EditorSDLWindowImpl, Glory::Editor::EditorOpenGLRenderImpl>(pGame);
        //editorApp.Run(pGame);
        //editorApp.Destroy();

        pGame.Destroy();

        loader.Unload();
    }

    _CrtDumpMemoryLeaks();

    return 0;
}