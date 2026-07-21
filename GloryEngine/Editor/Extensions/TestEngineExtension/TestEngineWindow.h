#pragma once
#include <EditorWindow.h>

namespace Glory::Editor
{
    class TestEngineWindow : public EditorWindowTemplate<TestEngineWindow>
    {
    public:
        TestEngineWindow();

    private:
        void DrawWindow() override;
        void OnGUI() override {}
    };
}
