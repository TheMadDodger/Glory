#include "GSceneEditor.h"
#include "EditorApplication.h"
#include "EditorSceneManager.h"
#include "EditorUI.h"

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
                    sampleRadius.Set(10.0f);

                auto sampleBias = ssao["SampleBias"];
                if (!sampleBias.Exists())
                    sampleBias.Set(0.0025f);

                auto kernelSize = ssao["KernelSize"];
                if (!kernelSize.Exists())
                    kernelSize.Set(64);

                auto blurType = ssao["BlurType"];
                if (!blurType.Exists())
                    blurType.SetEnum<BlurType>(BlurType::Box);

                auto blurSize = ssao["BlurSize"];
                if (!blurSize.Exists())
                    blurSize.Set(4);

                auto separation = ssao["Separation"];
                if (!separation.Exists())
                    separation.Set(1.0f);

                auto binsSize = ssao["BinsSize"];
                if (!binsSize.Exists())
                    binsSize.Set(5);

                ImGui::TextUnformatted("Generation");
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
            }
            ImGui::EndDisabled();
        }

        if (change)
        {
            sceneManager.SetSceneDirty((GScene*)m_pTarget);
        }

        return change;
    }
}
