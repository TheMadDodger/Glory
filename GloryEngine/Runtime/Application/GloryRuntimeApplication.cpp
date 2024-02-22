#include "pch.h"

#include <Console.h>
#include <Logs.h>
#include <WindowsDebugConsole.h>
#include <GloryRuntime.h>
#include <CommandLine.h>

int main(int argc, char* argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    {
        Glory::CommandLine commandLine{ argc, argv };
        std::string path = "";
        commandLine.GetValue("path", path);

        Glory::WindowCreateInfo windowCreateInfo;
        windowCreateInfo.WindowName = "GloryRuntime";
        windowCreateInfo.Width = 2560;
        windowCreateInfo.Height = 1300;
        //windowCreateInfo.WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
        windowCreateInfo.WindowFlags = 2 | 32;

        Glory::Console console;
        Glory::Debug debug{ &console };

        console.RegisterConsole<Glory::Logs>();
        console.RegisterConsole<Glory::WindowsDebugConsole>();

        Glory::EngineLoader engineLoader("./Modules", windowCreateInfo);
        Glory::Engine engine = engineLoader.LoadEngineFromPath(&console, &debug);
        std::filesystem::path moduleSettingsRootPath = "./Modules";
        engine.LoadModuleSettings(moduleSettingsRootPath);

        Glory::GloryRuntime runtime{ &engine };
        runtime.Initialize();

        if (!path.empty())
            runtime.LoadScene(path);

        runtime.Run();
    }

    _CrtDumpMemoryLeaks();

    return 0;
}