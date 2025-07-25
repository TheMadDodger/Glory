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
        windowCreateInfo.WindowFlags = W_Resizeable;
        windowCreateInfo.Fullscreen = true;
        windowCreateInfo.Maximize = false;

        static Glory::Logs logs;
        static Glory::WindowsDebugConsole windowsConsole(m_Console.get());

        m_Console->RegisterConsole(&logs);
        m_Console->RegisterConsole(&windowsConsole);

        m_EngineLoader.reset(new EngineLoader("./Modules"));

        EngineCreateInfo info = m_EngineLoader->LoadEngineInfoFromPath(m_Console.get(), m_Debug.get());
        m_Engine.reset(new Engine(info));
        m_Console->SetEngine(m_Engine.get());
        std::filesystem::path moduleSettingsRootPath = dataPath;
        moduleSettingsRootPath = moduleSettingsRootPath.parent_path();
        moduleSettingsRootPath.append("Modules/Config");
        m_Engine->LoadModuleSettings(moduleSettingsRootPath);
        m_Engine->SetMainWindowInfo(std::move(windowCreateInfo));

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
