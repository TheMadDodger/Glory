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

    private:
        virtual void Initialize() override;

    private:
        ImGuiTestEngine* m_pEngine;
    };
}
