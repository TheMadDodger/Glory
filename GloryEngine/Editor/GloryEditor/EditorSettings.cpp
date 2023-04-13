#include "EditorSettings.h"
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
        WindowModule* pWindowModule = pEngine->GetWindowModule();
        Window* pMainWindow = pWindowModule->GetMainWindow();
        if (!pMainWindow) return;

        YAML::Emitter emitter;
        emitter << YAML::BeginMap;

        emitter << YAML::Key << "Window";

        NodeValueRef window = m_YAMLFile["Window"];
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
        Debug::LogInfo(stream.str());
    }

    NodeValueRef EditorSettings::operator[](const std::filesystem::path& path)
    {
        return m_YAMLFile[path];
    }

    void EditorSettings::LoadSettingsFile(Engine* pEngine)
    {
        WindowModule* pWindowModule = pEngine->GetWindowModule();
        WindowCreateInfo* pMainWindowSettings = pWindowModule->GetMainWindowCreateInfo();

        NodeValueRef window = m_YAMLFile["Window"];
        if (window.IsMap())
        {
            pMainWindowSettings->Width = window["Width"].As<uint32_t>(1920);
            pMainWindowSettings->Height = window["Height"].As<uint32_t>(1080);

            const glm::vec2 pos = window["Position"].As<glm::vec2>({0, 0});

            Window* pMainWindow = pWindowModule->GetMainWindow();
            int top;
            pMainWindow->GetWindowBordersSize(&top, nullptr, nullptr, nullptr);

            pMainWindow->Resize(pMainWindowSettings->Width, pMainWindowSettings->Height);
            pMainWindow->SetPosition((int)pos.x, (int)pos.y + top);
        }

        Shortcuts::LoadShortcuts(m_YAMLFile);

        std::stringstream stream;
        stream << "Loaded editor settings from " << m_YAMLFile.Path().string();
        Debug::LogInfo(stream.str());
    }

    void EditorSettings::LoadDefaultSettings(Engine* pEngine)
    {
        WindowModule* pWindowModule = pEngine->GetWindowModule();
        WindowCreateInfo* pMainWindowSettings = pWindowModule->GetMainWindowCreateInfo();
        pWindowModule->GetCurrentScreenResolution(pMainWindowSettings->Width, pMainWindowSettings->Height);
        Window* pMainWindow = pWindowModule->GetMainWindow();
        pMainWindow->Resize(pMainWindowSettings->Width, pMainWindowSettings->Height);
        pMainWindow->SetPosition(0, 0);

        Debug::LogInfo("Loaded default editor settings");
    }
}
