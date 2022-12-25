#pragma once
#include "EditorWindow.h"
#include <yaml-cpp/yaml.h>
#include <ProjectSettings.h>

namespace Glory::Editor
{
    class ProjectSettingsWindow : public EditorWindowTemplate<ProjectSettingsWindow>
    {
    public:
        ProjectSettingsWindow();
        virtual ~ProjectSettingsWindow();

    private:
        virtual void OnGUI() override;
        virtual void OnOpen() override;

    private:
        static const char* TAB_NAMES[];
        ProjectSettingsType m_CurrentTab;
    };
}
