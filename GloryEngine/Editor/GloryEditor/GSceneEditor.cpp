#include "GSceneEditor.h"
#include "EditorApplication.h"
#include "EditorSceneManager.h"
#include "EditorUI.h"
#include "AssetCompiler.h"
#include "AssetPicker.h"
#include "Undo.h"

#include <Debug.h>
#include <CubemapData.h>

namespace Glory::Editor
{
    bool GSceneEditor::OnGUI()
    {
        bool change = false;

        EditorSceneManager& sceneManager = EditorApplication::GetInstance()->GetSceneManager();
        auto sceneFile = sceneManager.GetSceneFile(m_pTarget->GetUUID());
        if (!sceneFile) return false;

        auto yamlFile = **sceneFile;
        auto settings = yamlFile["Settings"];
        if (!settings.Exists() || !settings.IsMap())
            settings.SetMap();

        auto rendering = settings["Rendering"];
        if (!rendering.Exists() || !rendering.IsMap())
            rendering.SetMap();

        if (EditorUI::Header("Rendering"))
        {
            if (EditorUI::HeaderLight("Lighting"))
            {
                auto lighting = rendering["Lighting"];
                auto skybox = lighting["Skybox"];
                auto irradiance = lighting["Irradiance"];
                UUID oldValue = skybox.As<uint64_t>(0);
                UUID newValue = oldValue;
                if (AssetPicker::ResourceDropdown("Skybox", ResourceTypes::GetHash<CubemapData>(), &newValue))
                {
                    Undo::ApplyYAMLEdit(yamlFile, skybox.Path(), uint64_t(oldValue), uint64_t(newValue));
                    change = true;
                }

                oldValue = irradiance.As<uint64_t>(0);
                newValue = oldValue;
                if (AssetPicker::ResourceDropdown("Irradiance Map", ResourceTypes::GetHash<CubemapData>(), &newValue))
                {
                    Undo::ApplyYAMLEdit(yamlFile, irradiance.Path(), uint64_t(oldValue), uint64_t(newValue));
                    change = true;
                }
            }

            auto ssao = rendering["SSAO"];
            if (!ssao.Exists() || !ssao.IsMap())
                ssao.SetMap();

            auto enable = ssao["Enable"];
            if (!enable.Exists())
                enable.Set(true);

            bool open = false;
            change |= EditorUI::HeaderWithCheckbox("SSAO", open, yamlFile, enable.Path());

            ImGui::BeginDisabled(!enable.As<bool>());
            if (open)
            {
                auto sampleRadius = ssao["SampleRadius"];
                if (!sampleRadius.Exists())
                    sampleRadius.Set(DefaultSSAO.m_SampleRadius);

                auto sampleBias = ssao["SampleBias"];
                if (!sampleBias.Exists())
                    sampleBias.Set(DefaultSSAO.m_SampleBias);

                auto kernelSize = ssao["KernelSize"];
                if (!kernelSize.Exists())
                    kernelSize.Set(DefaultSSAO.m_KernelSize);

                auto blurType = ssao["BlurType"];
                if (!blurType.Exists())
                    blurType.SetEnum<BlurType>(DefaultSSAO.m_BlurType);

                auto blurSize = ssao["BlurSize"];
                if (!blurSize.Exists())
                    blurSize.Set(DefaultSSAO.m_BlurSize);

                auto separation = ssao["Separation"];
                if (!separation.Exists())
                    separation.Set(DefaultSSAO.m_Separation);

                auto binsSize = ssao["BinsSize"];
                if (!binsSize.Exists())
                    binsSize.Set(DefaultSSAO.m_BinsSize);

                auto magnitude = ssao["Magnitude"];
                if (!magnitude.Exists())
                    magnitude.Set(DefaultSSAO.m_Magnitude);

                auto contrast = ssao["Contrast"];
                if (!contrast.Exists())
                    contrast.Set(DefaultSSAO.m_Contrast);

                ImGui::TextUnformatted("Render");
                change |= EditorUI::InputFloat(yamlFile, sampleRadius.Path(), 0.0f);
                change |= EditorUI::InputFloat(yamlFile, sampleBias.Path(), 0.0f, 10.0f, 0.0001f);
                change |= EditorUI::InputInt(yamlFile, kernelSize.Path(), 0);

                ImGui::Separator();
                ImGui::TextUnformatted("Blur");
                change |= EditorUI::InputEnum<BlurType>(yamlFile, blurType.Path());
                const BlurType selectedBlur = blurType.AsEnum<BlurType>();

                switch (selectedBlur)
                {
                case BlurType::Box:
                    change |= EditorUI::InputInt(yamlFile, blurSize.Path(), 0);
                    change |= EditorUI::InputFloat(yamlFile, separation.Path(), 0.0f, 1.0f, 0.01f);
                    break;
                case BlurType::Median:
                    change |= EditorUI::InputInt(yamlFile, blurSize.Path(), 0);
                    change |= EditorUI::InputInt(yamlFile, binsSize.Path(), 0);
                    break;
                default:
                    break;
                }

                ImGui::Separator();
                ImGui::TextUnformatted("Compose");
                change |= EditorUI::InputFloat(yamlFile, magnitude.Path(), 0.0f);
                change |= EditorUI::InputFloat(yamlFile, contrast.Path(), 0.0f);
            }
            ImGui::EndDisabled();
        }

        if (change)
        {
            sceneManager.SetSceneDirty((GScene*)m_pTarget);
            if (!AssetCompiler::CompileSceneSettings(m_pTarget->GetUUID()))
                sceneManager.GetEngine()->GetDebug().LogError("Failed to re-compile scene settings");
        }

        return change;
    }
}
