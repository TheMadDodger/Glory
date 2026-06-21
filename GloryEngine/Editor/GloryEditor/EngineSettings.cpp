#include "ProjectSettings.h"
#include "EditorApplication.h"
#include "EditorPlatform.h"
#include "PropertyDrawer.h"
#include "EditorUI.h"

#include <imgui.h>
#include <IEngine.h>
#include <SettingsContainer.h>
#include <PropertySerializer.h>
#include <Debug.h>
#include <GloryAssert.h>

#include <IconsFontAwesome6.h>

#include <fstream>

namespace Glory::Editor
{
    Utils::YAMLFileRef& EngineSettings::GetModuleSettingsFile(Module* pModule)
    {
        const ModuleMetaData& moduleMetaData = pModule->GetMetaData();
        auto iter = m_SettingFiles.find(moduleMetaData.Name());
        if (iter == m_SettingFiles.end())
        {
            std::filesystem::path moduleSettingsPath = ProjectSpace::GetOpenProject()->RootPath();
            moduleSettingsPath.append("Modules").append(moduleMetaData.Name() + ".module");
            iter = m_SettingFiles.emplace(moduleMetaData.Name(), moduleSettingsPath).first;
        }
        return iter->second;
    }

	bool EngineSettings::OnGui()
	{
        DrawLeftPanel();
        return DrawRightPanel();
	}

    void EngineSettings::OnSave(ProjectSpace* pProject)
    {
        IEngine* pEngine = EditorApplication::GetInstance()->GetEngine();

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

        for (auto& [name, file] : m_SettingFiles)
            file.Save();
    }

    void EngineSettings::OnSettingsLoaded()
    {
        Undo::RegisterChangeHandler(".module", "", [this](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			OnModuleSettingsChanged(file, path);
		});
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

        IEngine* pEngine = EditorApplication::GetInstance()->GetEngine();

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

        IEngine* pEngine = EditorApplication::GetInstance()->GetEngine();
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

        SettingsBase* pSettings = pModule->GetSettings();
        if (pSettings)
        {
            auto& file = GetModuleSettingsFile(pModule);
            change |= DrawSettings(*pSettings, file);
        }
        else
        {
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
                    auto valueNode = settingsNode[value];
                    const bool isDefault = settings.IsSetToDefault(value);

                    const float start = ImGui::GetCursorPosX();
                    const float totalWidth = ImGui::GetContentRegionAvail().x;

                    if (!isDefault)
                    {
                        EditorUI::PushFlag(EditorUI::Flag::HasSmallButton);
                        EditorUI::RemoveButtonPadding = 32.0f;
                    }
                    switch (type)
                    {
                    case ST_Enum:
                    case ST_Asset:
                        change |= PropertyDrawer::GetPropertyDrawer(type)->Draw(value, valueNode, elementType, 0);
                        break;
                    default:
                        change |= PropertyDrawer::DrawProperty(value, settingsNode, type, elementType, 0);
                        break;
                    }
                    if (!isDefault)
                    {
                        EditorUI::RemoveButtonPadding = 24.0f;
                        EditorUI::PopFlag();
                        ImGui::SameLine();
                        if (ImGui::Button(ICON_FA_ARROW_ROTATE_LEFT, { 24.0f, 24.0f }))
                        {
                            settings.ResetToDefault(value);
                            change = true;
                        }
                    }
                }
            }
            if (change)
                settings.SetDirty();
        }

        ImGui::EndChild();

        return change;
    }

    bool EngineSettings::DrawSettings(SettingsBase& settings, Utils::YAMLFileRef& file)
    {
        bool change = false;
        void* data = *settings;
        const Utils::Reflect::TypeData* type = settings.GetType();
        Undo::StartRecord("Engine Settings");
        for (size_t i = 0; i < type->FieldCount(); ++i)
        {
            auto field = type->GetFieldData(i);
            const std::filesystem::path propPath{ field->Name() };
            auto flags = Reflect::GetFieldFlags(field);

            std::stringstream str;
            str << type->TypeName() << "::" << field->Name();
            const std::string_view group = settings.GetGroup(str.str());

            if (!group.empty())
            {
                ImGui::PushFont(EditorPlatform::LargeFont);
                ImGui::TextUnformatted(group.data());
                ImGui::PopFont();
            }

            change |= PropertyDrawer::DrawProperty(file, propPath, field->Type(), field->ArrayElementType(),
                flags, field->DisplayName(), field->Description());
        }
        Undo::StopRecord();
        return change;
    }

    void EngineSettings::OnModuleSettingsChanged(Utils::YAMLFileRef& file, const std::filesystem::path& path)
    {
        const std::string moduleName = file.Path().filename().replace_extension("").string();
        EditorApplication* pApp = EditorApplication::GetInstance();
        IEngine* pEngine = pApp->GetEngine();
        Module* pModule = pEngine->GetModule(moduleName);
        if (!pModule)
        {
            std::stringstream str;
            str << "EngineSettings::OnModuleSettingsChanged > Unknown module " << moduleName;
            pEngine->GetDebug().LogWarning(str.str());
            return;
        }

        SettingsBase& settings = *pModule->GetSettings();
        auto parentType = settings.GetType();
        auto type = parentType;
        void* data = *settings;
        auto field = type->GetFieldData(0);
        for (auto iter = path.begin(); iter != path.end(); ++iter)
        {
            const std::string pathComp = iter->string();
            if (field && field->Type() == ST_Array)
            {
                std::stringstream str;
                str << pathComp;
                size_t index = 0;
                str >> index;
                data = Reflect::ElementAddress(data, field->ArrayElementType(), index);
                parentType = type;
                type = Reflect::GetTyeData(field->ArrayElementType());
                field = field->GetArrayElementFieldData(index);
                continue;
            }

            field = type->GetFieldData(pathComp);
            GLORY_ASSERT(field, "Field is null");
            data = field->GetAddress(data);
            parentType = type;
            type = Reflect::GetTyeData(field->ArrayElementType());
        }

        if (!field)
            return;

        auto prop = file[path];
        GLORY_ASSERT(field, "Field is null");
        pApp->GetSerializers().DeserializeProperty(field, data, prop);

        std::stringstream str;
        str << parentType->TypeName() << "::" << field->Name();
        settings.NotifyChange(str.str());
    }
}
