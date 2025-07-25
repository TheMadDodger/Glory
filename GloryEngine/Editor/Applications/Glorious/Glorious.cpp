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
        windowCreateInfo.Width = 0.0f;
        windowCreateInfo.Height = 0.0f;
        windowCreateInfo.WindowFlags = Glory::WindowFlags::W_Resizeable;
        windowCreateInfo.Fullscreen = false;
        windowCreateInfo.Maximize = true;

        Glory::Console console;
        Glory::Debug debug{ &console };

        Glory::Logs logs;
        Glory::WindowsDebugConsole windowsConsole(&console);

        console.RegisterConsole(&logs);
        console.RegisterConsole(&windowsConsole);

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

        Glory::EngineLoader engineLoader(engineConfPath);
        Glory::Engine engine = engineLoader.LoadEngine(&console, &debug);
        console.SetEngine(&engine);
        std::filesystem::path moduleSettingsRootPath = engineConfPath.parent_path().parent_path();
        moduleSettingsRootPath.append("Modules");
        engine.LoadModuleSettings(moduleSettingsRootPath);
        engine.SetMainWindowInfo(std::move(windowCreateInfo));

        Glory::EditorLoader editorLoader;
        Glory::EditorCreateInfo editorCreateInfo = editorLoader.LoadEditor(&engine, engineLoader);

        Glory::Editor::EditorApplication application(editorCreateInfo);
        application.Initialize();
        application.Start(projectPath);
        application.Run();
    }

    _CrtDumpMemoryLeaks();

    return 0;
}