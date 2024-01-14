#define IMGUI_DEFINE_MATH_OPERATORS 

#include "DND.h"
#include <ResourceType.h>
#include <Hash.h>
#include <SerializedTypes.h>
#include <imgui_internal.h>


namespace Glory::Editor
{
    void DND::DragAndDropSource(const std::string_view name, void* payload, size_t payloadSize, std::function<void()> previewCallback, ImGuiDragDropFlags flags)
    {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload(name.data(), payload, payloadSize);
            previewCallback();
            ImGui::EndDragDropSource();
        }
    }

    bool DND::HandleDragAndDropTarget(std::function<void(uint32_t, const ImGuiPayload*)> callback)
    {
        if (!IsEnabled()) return true;
        if (ImGui::BeginDragDropTarget())
        {
            const bool claimed = HandleTargetInternal(callback);
            ImGui::EndDragDropTarget();
            return claimed;
        }
        return false;
    }

    bool DND::HandleDragAndDropWindowTarget(std::function<void(uint32_t, const ImGuiPayload*)> callback)
    {
        const ImVec2 windowPos = ImGui::GetWindowPos();
        const ImVec2 min = windowPos + ImGui::GetWindowContentRegionMin();
        const ImVec2 max = windowPos + ImGui::GetWindowContentRegionMax();
        const ImRect rect{ min, max };
        if (ImGui::BeginDragDropTargetCustom(rect, ImGui::GetCurrentWindow()->ID))
        {
            const bool claimed = HandleTargetInternal(callback);
            ImGui::EndDragDropTarget();
            return claimed;
        }

        return false;
    }

    bool DND::HandleTargetInternal(std::function<void(uint32_t, const ImGuiPayload*)> callback)
    {
        for (size_t i = 0; i < m_AcceptedTypes.size(); ++i)
        {
            const uint32_t type = m_AcceptedTypes[i];
            if (type == ST_Path)
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(STNames[ST_Path], m_DNDFlags);
                if (!payload) continue;
                callback(type, payload);
                return true;
            }

            ResourceType* pResource = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes().GetResourceType(type);
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(pResource->Name().c_str(), m_DNDFlags);
            if (!payload) continue;
            callback(type, payload);
            return true;
        }
        return false;
    }
}
