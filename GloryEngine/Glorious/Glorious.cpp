// Glorious.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "EditorLoader.h"
#include "CommandLine.h"
#include "ProjectLock.h"
#include <Console.h>
#include <ProjectSpace.h>

int main(int argc, char* argv[])
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

        Glory::GloryContext::CreateContext();

        Glory::CommandLine commandLine(argc, argv);

        std::string projectPath;
        if (!commandLine.GetValue("projectPath", projectPath))
        {
            Glory::Debug::LogError("Missing project path in launch arguments!");
            Glory::GloryContext::DestroyContext();
            return -1;
        }

        if (!std::filesystem::exists(projectPath))
        {
            Glory::Debug::LogError("Invalid project path!");
            Glory::GloryContext::DestroyContext();
            return -1;
        }

        Glory::Editor::ProjectLock lock(projectPath);
        if (!lock.Lock())
        {
            Glory::Debug::LogError("Project already open in another editor!");
            Glory::GloryContext::DestroyContext();
            return -1;
        }


        std::filesystem::path engineConfPath = projectPath;
        engineConfPath = engineConfPath.parent_path();
        engineConfPath.append("ProjectSettings").append("Engine.yaml");

        Glory::EngineLoader engineLoader(engineConfPath);
        Glory::Engine* pEngine = engineLoader.LoadEngine(windowCreateInfo);
        
        if (pEngine == nullptr)
        {
            Glory::Debug::LogError("The projects engine configuration could not be loaded!");
            Glory::GloryContext::DestroyContext();
            return -1;
        }

        Glory::GameSettings gameSettings;
        gameSettings.pEngine = pEngine;
        gameSettings.pGameState = new Glory::GameState();
        gameSettings.ApplicationType = Glory::ApplicationType::AT_Editor;
        Glory::Game& pGame = Glory::Game::CreateGame(gameSettings);
        pGame.Initialize();
        
        Glory::EditorLoader editorLoader;
        Glory::EditorCreateInfo editorCreateInfo = editorLoader.LoadEditor(pGame, engineLoader);
        
        Glory::Editor::EditorApplication application(editorCreateInfo);
        application.Initialize(pGame);

        Glory::Editor::ProjectSpace::OpenProject(projectPath);

        application.Run(pGame);
        
        application.Destroy();
        pGame.Destroy();
        engineLoader.Unload();
    }

    _CrtDumpMemoryLeaks();

    return 0;
}