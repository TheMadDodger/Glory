#pragma once
#include "test_engine_editor_visibility.h"

#include <filesystem>

#include <BaseEditorExtension.h>

EXTENSION_H(GLORY_TESTENGINE_EXTENSION_API)

struct ImGuiTestEngine;
struct ImGuiTestContext;

namespace YAML
{
    class Node;
}

namespace Glory::Editor
{
    class ProjectSpace;

    class TestEngineEditorExtension : public BaseEditorExtension
    {
    public:
        TestEngineEditorExtension();
        virtual ~TestEngineEditorExtension();

        GLORY_TESTENGINE_EXTENSION_API static ImGuiTestEngine* GetTestEngine();

        GLORY_TESTENGINE_EXTENSION_API static void RegisterTests(ProjectSpace* pProject);

    private:
        virtual void Initialize() override;
        virtual void Update() override;

        static void FindTestsRecursive(const std::filesystem::path& rootPath, const std::filesystem::path& path);


    private:
        UUID m_OnOpenProjectCallback = 0ull;
    };
}
