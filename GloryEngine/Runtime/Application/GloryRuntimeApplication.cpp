#include "pch.h"
#include "GloryRuntimeApplication.h"

#include <Console.h>
#include <Logs.h>
#include <WindowsDebugConsole.h>
#include <GloryRuntime.h>
#include <CommandLine.h>

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
        std::string path = "";
        commandLine.GetValue("path", path);

        Glory::WindowCreateInfo windowCreateInfo;
        windowCreateInfo.WindowName = "GloryRuntime";
        windowCreateInfo.Width = 2560;
        windowCreateInfo.Height = 1300;
        //windowCreateInfo.WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
        windowCreateInfo.WindowFlags = 2 | 32;

        m_Console->RegisterConsole<Glory::Logs>();
        m_Console->RegisterConsole<Glory::WindowsDebugConsole>();

        m_EngineLoader.reset(new EngineLoader("./Modules", windowCreateInfo));

        EngineCreateInfo info = m_EngineLoader->LoadEngineInfoFromPath(m_Console.get(), m_Debug.get());
        m_Engine.reset(new Engine(info));
        std::filesystem::path moduleSettingsRootPath = "./Modules/Config";
        m_Engine->LoadModuleSettings(moduleSettingsRootPath);

        m_Runtime.reset(new GloryRuntime(m_Engine.get()));
        m_Runtime->Initialize();

        if (!path.empty())
            m_Runtime->LoadScene(path);
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
