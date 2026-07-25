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

        using Operation = std::function<bool(const std::filesystem::path&, YAML::Node&, ImGuiTestContext*)>;
        GLORY_TESTENGINE_EXTENSION_API static ImGuiTestEngine* GetTestEngine();
        GLORY_TESTENGINE_EXTENSION_API static void RegisterOperation(std::string&& name, Operation operation);

        GLORY_TESTENGINE_EXTENSION_API static void RegisterTests(ProjectSpace* pProject);

    private:
        virtual void Initialize() override;
        virtual void Update() override;

        static void FindTestsRecursive(const std::filesystem::path& rootPath, const std::filesystem::path& path);


    private:
        UUID m_OnOpenProjectCallback = 0ull;
    };
}
