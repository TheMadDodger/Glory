#include "StandardPropertyDrawers.h"
#include <imgui.h>
#include <PropertyFlags.h>
#include <glm/gtc/quaternion.hpp>
#include <LayerManager.h>

namespace Glory::Editor
{
	bool FloatDrawer::OnGUI(const std::string& label, float* data, uint32_t flags) const
	{
		return ImGui::InputFloat(label.data(), data);
	}

	bool IntDrawer::OnGUI(const std::string& label, int* data, uint32_t flags) const
	{
		return ImGui::InputInt(label.data(), data);
	}

	bool BoolDrawer::OnGUI(const std::string& label, bool* data, uint32_t flags) const
	{
		return ImGui::Checkbox(label.data(), data);
	}

	bool DoubleDrawer::OnGUI(const std::string& label, double* data, uint32_t flags) const
	{
		return ImGui::InputDouble(label.data(), data);
	}

	bool Vector2Drawer::OnGUI(const std::string& label, glm::vec2* data, uint32_t flags) const
	{
		return ImGui::InputFloat2(label.data(), (float*)data);
	}

	bool Vector3Drawer::OnGUI(const std::string& label, glm::vec3* data, uint32_t flags) const
	{
		return ImGui::InputFloat3(label.data(), (float*)data);
	}

	bool Vector4Drawer::OnGUI(const std::string& label, glm::vec4* data, uint32_t flags) const
	{
		if (flags & Color)
		{
            ImVec4 color = ImVec4(data->x, data->y, data->z, data->w);

            ImGuiColorEditFlags misc_flags = (flags & HDR ? ImGuiColorEditFlags_HDR : 0); //| /**(drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop)*/ | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);
            static ImVec4 backup_color;
            bool open_popup = ImGui::ColorButton(label.data(), color, misc_flags);
            ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::Text(label.data());

            std::string colorPickerLabel = label + "_colorpicker";
            std::string pickerLabel = "##" + label + "_picker";

            if (open_popup)
            {
                ImGui::OpenPopup(colorPickerLabel.data());
                backup_color = color;
            }
            if (ImGui::BeginPopup(colorPickerLabel.data()))
            {
                ImGui::Text(label.data());
                ImGui::Separator();
                ImGui::ColorPicker4(pickerLabel.data(), (float*)&color, misc_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
                ImGui::SameLine();

                ImGui::BeginGroup(); // Lock X position
                ImGui::Text("Current");
                ImGui::ColorButton("##current", color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
                ImGui::Text("Previous");
                if (ImGui::ColorButton("##previous", backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
                    color = backup_color;
                ImGui::Separator();
                ImGui::Text("Palette");
                //for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
                //{
                //    ImGui::PushID(n);
                //    if ((n % 8) != 0)
                //        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);
                //
                //    ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
                //    if (ImGui::ColorButton("##palette", saved_palette[n], palette_button_flags, ImVec2(20, 20)))
                //        color = ImVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w); // Preserve alpha!
                //
                //    // Allow user to drop colors into each palette entry. Note that ColorButton() is already a
                //    // drag source by default, unless specifying the ImGuiColorEditFlags_NoDragDrop flag.
                //    if (ImGui::BeginDragDropTarget())
                //    {
                //        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
                //            memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
                //        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
                //            memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 4);
                //        ImGui::EndDragDropTarget();
                //    }
                //
                //    ImGui::PopID();
                //}
                ImGui::EndGroup();
                ImGui::EndPopup();

                data->x = color.x;
                data->y = color.y;
                data->z = color.z;
                data->w = color.w;
            }
			return true;
		}

		return ImGui::InputFloat4(label.data(), (float*)data);
	}

    bool QuatDrawer::OnGUI(const std::string& label, glm::quat* data, uint32_t flags) const
    {
        glm::vec3 euler = glm::eulerAngles(*data) / 3.141592f * 180.0f;
        if (ImGui::InputFloat3("Rotation", (float*)&euler, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
        {
            glm::quat q = glm::quat(euler * 3.141592f / 180.0f);
            data->x = q.x;
            data->y = q.y;
            data->z = q.z;
            data->w = q.w;
            return true;
        }
        return false;
    }

    bool LayerMaskDrawer::OnGUI(const std::string& label, LayerMask* data, uint32_t flags) const
    {
        std::vector<std::string> layerOptions;
        std::string layerText;

        layerOptions.clear();
        LayerManager::GetAllLayerNames(layerOptions);
        layerText = LayerManager::LayerMaskToString(*data);

        if (ImGui::BeginCombo("Layer Mask", layerText.data()))
        {
            for (size_t i = 0; i < layerOptions.size(); i++)
            {
                const Layer* pLayer = LayerManager::GetLayerAtIndex(i - 1);

                bool selected = pLayer == nullptr ? *data == 0 : (*data & pLayer->m_Mask) > 0;
                if (ImGui::Selectable(layerOptions[i].data(), selected))
                {
                    if (pLayer == nullptr)
                        *data = 0;
                    else
                        *data ^= pLayer->m_Mask;

                    layerText = LayerManager::LayerMaskToString(*data);
                }
            }

            ImGui::EndCombo();
        }

        return true;
    }
}