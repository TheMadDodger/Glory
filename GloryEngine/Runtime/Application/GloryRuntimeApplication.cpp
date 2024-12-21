#include "pch.h"
#include "GloryRuntimeApplication.h"

#include <Console.h>
#include <Logs.h>
#include <WindowsDebugConsole.h>
#include <GloryRuntime.h>
#include <CommandLine.h>
#include <AssetDatabase.h>

namespace Glory
{
    GloryRuntimeApplication::GloryRuntimeApplication()
    {
    }

    GloryRuntimeApplication::GloryRuntimeApplication(const std::string_view appName):
        m_AppName(appName)
    {
    }

    GloryRuntimeApplication::~GloryRuntimeApplication() = default;

    void GloryRuntimeApplication::SetName(const std::string_view appName)
    {
        m_AppName = appName;
    }

    void GloryRuntimeApplication::Initialize(int argc, char* argv[])
    {
        m_Console.reset(new Console());
        m_Debug.reset(new Debug(m_Console.get()));

        CommandLine commandLine{ argc, argv };
        std::string cmd = "";
        std::string dataPath = "./Data";
        commandLine.GetValue("command", cmd);
        if (!commandLine.GetValue("dataPath", dataPath))
            dataPath = "./Data";

        Glory::WindowCreateInfo windowCreateInfo;
        windowCreateInfo.WindowName = m_AppName;
        windowCreateInfo.Width = 0.0f;
        windowCreateInfo.Height = 0.0f;
        //windowCreateInfo.WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
        windowCreateInfo.WindowFlags = 2 | 32;
        windowCreateInfo.Fullscreen = true;
        windowCreateInfo.Maximize = false;

        m_Console->RegisterConsole<Glory::Logs>();
        m_Console->RegisterConsole<Glory::WindowsDebugConsole>();

        m_EngineLoader.reset(new EngineLoader("./Modules", windowCreateInfo));

        EngineCreateInfo info = m_EngineLoader->LoadEngineInfoFromPath(m_Console.get(), m_Debug.get());
        m_Engine.reset(new Engine(info));
        std::filesystem::path moduleSettingsRootPath = "./Modules/Config";
        m_Engine->LoadModuleSettings(moduleSettingsRootPath);

        m_Runtime.reset(new GloryRuntime(m_Engine.get()));
        m_Runtime->SetDataPath(dataPath);
        m_Runtime->Initialize();

        std::string frameLimitStr;
        if (commandLine.GetValue("frameLimit", frameLimitStr))
        {
            const float frameLimit = std::stof(frameLimitStr);
            m_Runtime->SetFramerateLimit(frameLimit);
        }

        if (!cmd.empty())
            m_Runtime->GetEngine()->GetConsole().ExecuteCommand(cmd);

        const UUID entryScene = m_Runtime->GetEngine()->GetAssetDatabase().GetEntryScene();
        if (entryScene)
            m_Runtime->LoadScene(entryScene);

        /* Ensure 3 second wait for splash screen */
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    void GloryRuntimeApplication::Run()
    {
        m_Runtime->Run();
    }

    void GloryRuntimeApplication::Cleanup()
    {
        m_EngineLoader->Unload();
    }

    GloryRuntime& GloryRuntimeApplication::Runtime()
    {
        return *m_Runtime;
    }
}
