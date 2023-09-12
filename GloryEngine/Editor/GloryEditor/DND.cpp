#include "DND.h"
#include <ResourceType.h>
#include <Hash.h>
#include <SerializedTypes.h>

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

                ResourceType* pResource = ResourceType::GetResourceType(type);
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(pResource->Name().c_str(), m_DNDFlags);
                if (!payload) return false;
                callback(type, payload);
                return true;
            }
            ImGui::EndDragDropTarget();
        }
        return false;
    }
}
