#include "EditorSettings.h"
#include "EditorApplication.h"

#include <Debug.h>
#include <WindowModule.h>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <Object.h>
#include <fstream>
#include <YAML_GLM.h>
#include <Shortcuts.h>

namespace Glory::Editor
{
    EditorSettings::EditorSettings(std::filesystem::path editorSettingsPath) : m_YAMLFile(editorSettingsPath)
    {
    }

    EditorSettings::~EditorSettings()
    {
    }

    void EditorSettings::Load(Engine* pEngine)
	{
        if (std::filesystem::exists(m_YAMLFile.Path()))
        {
            LoadSettingsFile(pEngine);
            return;
        }
        LoadDefaultSettings(pEngine);
	}

    void EditorSettings::Save(Engine* pEngine)
    {
        WindowModule* pWindowModule = pEngine->GetMainModule<WindowModule>();
        Window* pMainWindow = pWindowModule->GetMainWindow();
        if (!pMainWindow) return;

        YAML::Emitter emitter;
        emitter << YAML::BeginMap;

        emitter << YAML::Key << "Window";

        Utils::NodeValueRef window = m_YAMLFile["Window"];
        if (!window.IsMap())
        {
            window.Set(YAML::Node(YAML::NodeType::Map));
        }

        int width, height, x, y;
        pMainWindow->GetWindowSize(&width, &height);
        pMainWindow->GetPosition(&x, &y);

        window["Width"].Set((uint32_t)width);
        window["Height"].Set((uint32_t)height);
        window["Position"].Set(glm::vec2(x, y));

        Shortcuts::SaveShortcuts(m_YAMLFile);
        m_YAMLFile.Save();

        std::stringstream stream;
        stream << "Saved editor settings to " << m_YAMLFile.Path().string();
        EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo(stream.str());
    }

    Utils::NodeValueRef EditorSettings::operator[](const std::filesystem::path& path)
    {
        return m_YAMLFile[path];
    }

    void EditorSettings::LoadSettingsFile(Engine* pEngine)
    {
        WindowModule* pWindowModule = pEngine->GetMainModule<WindowModule>();
        WindowCreateInfo& mainWindowSettings = pEngine->MainWindowInfo();

        Utils::NodeValueRef window = m_YAMLFile["Window"];
        if (window.IsMap())
        {
            mainWindowSettings.Width = window["Width"].As<uint32_t>(1920);
            mainWindowSettings.Height = window["Height"].As<uint32_t>(1080);

            const glm::vec2 pos = window["Position"].As<glm::vec2>({0, 0});

            Window* pMainWindow = pWindowModule->GetMainWindow();
            pMainWindow->Resize(mainWindowSettings.Width, mainWindowSettings.Height);
            pMainWindow->SetPosition((int)pos.x, (int)pos.y);
        }

        Shortcuts::LoadShortcuts(m_YAMLFile);

        std::stringstream stream;
        stream << "Loaded editor settings from " << m_YAMLFile.Path().string();
        EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo(stream.str());
    }

    void EditorSettings::LoadDefaultSettings(Engine* pEngine)
    {
        WindowModule* pWindowModule = pEngine->GetMainModule<WindowModule>();
        WindowCreateInfo& mainWindowSettings = pEngine->MainWindowInfo();
        pWindowModule->GetCurrentScreenResolution(mainWindowSettings.Width, mainWindowSettings.Height);
        Window* pMainWindow = pWindowModule->GetMainWindow();
        pMainWindow->Resize(mainWindowSettings.Width, mainWindowSettings.Height);
        pMainWindow->SetPosition(0, 0);

        EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo("Loaded default editor settings");
    }
}
