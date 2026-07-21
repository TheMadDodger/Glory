#pragma once
#include "test_engine_editor_visibility.h"

#include <BaseEditorExtension.h>

#include <imgui_te_engine.h>

EXTENSION_H(GLORY_TESTENGINE_EXTENSION_API)

namespace Glory::Editor
{
    class TestEngineEditorExtension : public BaseEditorExtension
    {
    public:
        TestEngineEditorExtension();
        virtual ~TestEngineEditorExtension();

        GLORY_TESTENGINE_EXTENSION_API static ImGuiTestEngine* GetTestEngine();

    private:
        virtual void Initialize() override;

        void RegisterTests();
    };
}
