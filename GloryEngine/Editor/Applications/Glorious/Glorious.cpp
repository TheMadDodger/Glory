// Glorious.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "EditorLoader.h"
#include "CommandLine.h"
#include "ProjectLock.h"

#include <Console.h>
#include <Logs.h>
#include <ProjectSpace.h>
#include <WindowsDebugConsole.h>

int main(int argc, char* argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    {
        Glory::WindowCreateInfo windowCreateInfo;
        windowCreateInfo.WindowName = "Glorious";
        windowCreateInfo.Width = 2560;
        windowCreateInfo.Height = 1300;
        //windowCreateInfo.WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
        windowCreateInfo.WindowFlags = 2 | 32;

        Glory::Console console;
        Glory::Debug debug{ &console };

        console.RegisterConsole<Glory::Logs>();
        console.RegisterConsole<Glory::WindowsDebugConsole>();

        Glory::CommandLine commandLine(argc, argv);

        std::string projectPath;
        if (!commandLine.GetValue("projectPath", projectPath))
        {
            debug.LogError("Missing project path in launch arguments!");
            return -1;
        }

        if (!std::filesystem::exists(projectPath))
        {
            debug.LogError("Invalid project path!");
            return -1;
        }

        Glory::Editor::ProjectLock lock(projectPath);
        if (!lock.Lock())
        {
            debug.LogError("Project already open in another editor!");
            return -1;
        }

        std::filesystem::path engineConfPath = projectPath;
        engineConfPath = engineConfPath.parent_path();
        engineConfPath.append("ProjectSettings").append("Engine.yaml");

        Glory::EngineLoader engineLoader(engineConfPath, windowCreateInfo);
        Glory::Engine engine = engineLoader.LoadEngine(&console, &debug);
        std::filesystem::path moduleSettingsRootPath = engineConfPath.parent_path().parent_path();
        moduleSettingsRootPath.append("Modules");
        engine.LoadModuleSettings(moduleSettingsRootPath);

        //if (pEngine == nullptr)
        //{
        //    Glory::m_pEngine->GetDebug().LogError("The projects engine configuration could not be loaded!");
        //    Glory::GloryContext::DestroyContext();
        //    return -1;
        //}

        Glory::EditorLoader editorLoader;
        Glory::EditorCreateInfo editorCreateInfo = editorLoader.LoadEditor(&engine, engineLoader);

        Glory::Editor::EditorApplication application(editorCreateInfo);
        application.Initialize();

        Glory::Editor::ProjectSpace::OpenProject(projectPath);

        application.Run();
    }

    _CrtDumpMemoryLeaks();

    return 0;
}