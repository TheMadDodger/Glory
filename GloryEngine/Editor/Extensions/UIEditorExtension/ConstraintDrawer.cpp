#include "ConstraintDrawer.h"

#include <Constraints.h>

#include <EditorUI.h>
#include <Undo.h>

namespace Glory::Editor
{
    bool DrawConstraintProperty(Utils::YAMLFileRef& file, const std::filesystem::path& path, uint32_t typeHash, uint32_t flags)
    {
        std::string label = path.filename().string().data();
        if (label == "Value")
            label = path.parent_path().filename().string();

        auto prop = file[path];
        auto constraintType = prop["m_Constraint"];
        auto constraintValue = prop["m_Value"];
        const uint32_t type = constraintType.As<uint32_t>();

        auto& names = Constraints::Names();
        const size_t oldIndex = Constraints::IndexOf(type);
        size_t index = oldIndex;

        bool change = false;

        ImGui::TextUnformatted(EditorUI::MakeCleanName(label).data());
        ImGui::PushID(label.data());
        ImGui::Separator();
        ImGui::Indent();
        if (EditorUI::InputDropdown("Constraint Type", names, &index, names[index]))
        {
            const uint32_t oldType = Constraints::Type(oldIndex);
            const uint32_t type = Constraints::Type(index);
            Undo::ApplyYAMLEdit(file, constraintType.Path(), oldType, type);
            change = true;
        }

        const float oldValue = constraintValue.As<float>();
        float newValue = oldValue;
        if (EditorUI::InputFloat(EditorUI::MakeCleanName(label), &newValue))
        {
            Undo::ApplyYAMLEdit(file, constraintValue.Path(), oldValue, newValue);
            change = true;
        }

        ImGui::Unindent();
        ImGui::Separator();
        ImGui::PopID();
        return change;
    }
}
