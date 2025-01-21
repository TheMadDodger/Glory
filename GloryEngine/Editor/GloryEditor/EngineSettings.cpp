#include "ProjectSettings.h"
#include "EditorApplication.h"

#include <imgui.h>
#include <Engine.h>
#include <EditorPlatform.h>
#include <PropertyDrawer.h>
#include <PropertySerializer.h>
#include <fstream>

namespace Glory::Editor
{
	bool EngineSettings::OnGui()
	{
        DrawLeftPanel();
        return DrawRightPanel();
	}

    void EngineSettings::OnSave(ProjectSpace* pProject)
    {
        Engine* pEngine = EditorApplication::GetInstance()->GetEngine();

        for (size_t i = 0; i < pEngine->ModulesCount(); i++)
        {
            Module* pModule = pEngine->GetModule(i);
            const std::string& moduleName = pModule->GetMetaData().Name();
            if (moduleName.empty()) continue;
            ModuleSettings& settings = pModule->Settings();
            std::filesystem::path settingsPath = pProject->ModuleSettingsPath();
            settingsPath.append(moduleName + ".yaml");
            YAML::Emitter out;
            out << settings.Node();
            std::ofstream outFile(settingsPath);
            outFile << out.c_str();
            outFile.close();
        }
    }

    void EngineSettings::DrawLeftPanel()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::PushFont(EditorPlatform::LargeFont);
        ImGui::BeginChild("LeftPanel", ImVec2(250.0f, 0.0f), true);
        ImGui::BeginChild("LeftPanelHeader", ImVec2(0.0f, 50.0f), false);

        ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();
        float size = EditorPlatform::LargeFont->FontSize;
        float cursorPosY = ImGui::GetCursorPosY();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (size / 2.0f) + (contentRegionAvail.y / 2.0f));
        ImGui::TextUnformatted("Glory Engine");
        ImGui::SetCursorPosY(cursorPosY);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + contentRegionAvail.x - contentRegionAvail.y);

        ImGui::EndChild();
        ImGui::Separator();
        ImGui::BeginChild("LeftPanelBody", ImVec2(0.0f, 0.0f), false);

        Engine* pEngine = EditorApplication::GetInstance()->GetEngine();

        for (size_t i = 0; i < pEngine->ModulesCount(); i++)
        {
            const std::string& moduleName = pEngine->GetModule(i)->GetMetaData().Name();
            if (moduleName.empty()) continue;
            bool selected = m_MenuIndex == i;
            ImGui::PushID(moduleName.data());
            if (ImGui::Selectable("##selectable", selected, 0, ImVec2(0.0f, 50.0f)))
            {
                m_MenuIndex = i;
            }

            const ImVec2 textSize = ImGui::CalcTextSize(moduleName.data());

            ImGui::SameLine();
            const ImVec2 cursorPos = ImGui::GetCursorPos();
            ImGui::SetCursorPos({ cursorPos.x + 5.0f, cursorPos.y + 25.0f - textSize.y / 2.0f });
            ImGui::TextUnformatted(moduleName.data());
            ImGui::PopID();
        }
        ImGui::PopFont();

        ImGui::EndChild();
        ImGui::EndChild();
    }

    bool EngineSettings::DrawRightPanel()
    {
        bool change = false;

        ImGui::SameLine();
        ImGui::BeginChild("RightPanel", ImVec2(), true);

        Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
        Module* pModule = pEngine->GetModule(m_MenuIndex);
        const ModuleMetaData& moduleMetaData = pModule->GetMetaData();
        const Glory::Version& version = pModule->ModuleVersion();
        std::string versionString;
        version.GetVersionString(versionString);

        ImGui::PushFont(EditorPlatform::LargeFont);
        ImGui::TextUnformatted(moduleMetaData.Name().data());
        ImGui::PopFont();
        ImGui::SameLine();
        const float availableWidth = ImGui::GetContentRegionAvail().x;
        const float cursorPosX = ImGui::GetCursorPosX();

        const float textWidth = ImGui::CalcTextSize(versionString.data()).x + ImGui::CalcTextSize("Version ").x;

        ImGui::SetCursorPosX(cursorPosX + availableWidth - textWidth);
        ImGui::Text("Version %s", versionString.data());

        ImGui::Separator();

        ImGui::Spacing();
        ModuleSettings& settings = pModule->Settings();
        YAML::Node& settingsNode = settings.Node();

        if (!settings.HasSettings())
        {
            ImGui::TextUnformatted("There are no settings for this module.");
            ImGui::EndChild();
            return change;
        }

        for (auto groupItor = settings.GroupsBegin(); groupItor != settings.GroupsEnd(); ++groupItor)
        {
            const std::string& group = *groupItor;

            ImGui::PushFont(EditorPlatform::LargeFont);
            ImGui::TextUnformatted(group.c_str());
            ImGui::PopFont();

            for (auto valueItor = settings.Begin(group); valueItor != settings.End(group); ++valueItor)
            {
                const std::string& value = *valueItor;
                const uint32_t type = settings.Type(value);
                const uint32_t elementType = settings.ElementType(value);
                switch (type)
                {
                case ST_Enum:
                case ST_Asset:
                    change |= PropertyDrawer::GetPropertyDrawer(type)->Draw(value, settingsNode[value], elementType, 0);
                    break;
                default:
                    change |= PropertyDrawer::DrawProperty(value, settingsNode, type, elementType, 0);
                    break;
                }
            }
        }

        ImGui::EndChild();

        return change;
    }
}