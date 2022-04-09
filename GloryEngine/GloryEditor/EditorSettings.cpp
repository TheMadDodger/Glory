#include "EditorSettings.h"
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <Object.h>
#include <fstream>
#include <YAML_GLM.h>

namespace Glory::Editor
{
    EditorSettings::EditorSettings(std::filesystem::path editorSettingsPath) : m_EditorSettingsPath(editorSettingsPath)
    {
    }

    EditorSettings::~EditorSettings()
    {
    }

    void EditorSettings::Load(Engine* pEngine)
	{
        if (std::filesystem::exists(m_EditorSettingsPath))
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

        int width, height, x, y;
        pMainWindow->GetWindowSize(&width, &height);
        pMainWindow->GetPosition(&x, &y);

        emitter << YAML::Value << YAML::BeginMap;
        emitter << YAML::Key << "Width";
        emitter << YAML::Value << (uint32_t)width;
        emitter << YAML::Key << "Height";
        emitter << YAML::Value << (uint32_t)height;
        emitter << YAML::Key << "Position";
        emitter << YAML::Value << glm::vec2(x, y);
        emitter << YAML::EndMap;

        emitter << YAML::EndMap;

        std::ofstream outStream(m_EditorSettingsPath);
        outStream << emitter.c_str();
        outStream.close();
    }

    void EditorSettings::LoadSettingsFile(Engine* pEngine)
    {
        WindowModule* pWindowModule = pEngine->GetWindowModule();
        WindowCreateInfo* pMainWindowSettings = pWindowModule->GetMainWindowCreateInfo();

        YAML::Node node = YAML::LoadFile(m_EditorSettingsPath.string());
        YAML::Node window = node["Window"];
        if (window.IsMap() && window.IsDefined())
        {
            YAML::Node subNodel;
            YAML_READ(window, subNodel, Width, pMainWindowSettings->Width, uint32_t);
            YAML_READ(window, subNodel, Height, pMainWindowSettings->Height, uint32_t);

            glm::vec2 pos;
            YAML_READ(window, subNodel, Position, pos, glm::vec2);

            Window* pMainWindow = pWindowModule->GetMainWindow();
            pMainWindow->Resize(pMainWindowSettings->Width, pMainWindowSettings->Height);
            pMainWindow->SetPosition(pos.x, pos.y);
        }
    }

    void EditorSettings::LoadDefaultSettings(Engine* pEngine)
    {
        WindowModule* pWindowModule = pEngine->GetWindowModule();
        WindowCreateInfo* pMainWindowSettings = pWindowModule->GetMainWindowCreateInfo();
        pWindowModule->GetCurrentScreenResolution(pMainWindowSettings->Width, pMainWindowSettings->Height);
        Window* pMainWindow = pWindowModule->GetMainWindow();
        pMainWindow->Resize(pMainWindowSettings->Width, pMainWindowSettings->Height);
        pMainWindow->SetPosition(0, 0);
    }
}
