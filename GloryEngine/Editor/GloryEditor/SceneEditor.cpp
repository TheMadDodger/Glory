#include "SceneEditor.h"

#include <imgui.h>

namespace Glory::Editor
{
    SceneEditor::SceneEditor() {}

    SceneEditor::~SceneEditor() {}

    bool SceneEditor::OnGUI()
    {
        ImGui::TextUnformatted("Scene editor!");

        return false;
    }
}