#include "AllowedDOFsPropertyDrawer.h"

#include <EditorUI.h>

namespace Glory::Editor
{
    template<>
    inline bool PropertyDrawerTemplate<AllowedDOFFlag>::OnGUI(const std::string& label, AllowedDOFFlag* data, uint32_t flags) const
    {
        ImGui::TextUnformatted(EditorUI::MakeCleanName(label).data());
        ImGui::Indent();
        bool change = EditorUI::CheckBoxFlags("Position", reinterpret_cast<uint32_t*>(data), { "x", "y", "z" },
            { uint32_t(AllowedDOFFlag::TranslationX), uint32_t(AllowedDOFFlag::TranslationY), uint32_t(AllowedDOFFlag::TranslationZ) });
        change |= EditorUI::CheckBoxFlags("Rotation", reinterpret_cast<uint32_t*>(data), { "x", "y", "z" },
            { uint32_t(AllowedDOFFlag::RotationX), uint32_t(AllowedDOFFlag::RotationY), uint32_t(AllowedDOFFlag::RotationZ) });
        ImGui::Unindent();
        return change;
    }
}

namespace YAML
{
    Emitter& YAML::operator<<(Emitter& out, const Glory::AllowedDOFFlag& value)
    {
        out << uint16_t(value);
        return out;
    }
}