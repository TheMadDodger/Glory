#include "EditorCSAPI.h"

#include <EditorUI.h>

#include <mono/jit/jit.h>

namespace Glory::Editor
{
    bool EditorUI_InputFloat(MonoString* label, float* value)
    {
        char* labelUtf8 = mono_string_to_utf8(label);
        return EditorUI::InputFloat(labelUtf8, value);
    }

    void LoadInternalCalls()
    {
        mono_add_internal_call("GloryEngine.Editor.EditorUI::EditorUI_InputFloat", EditorUI_InputFloat);
    }
}
