#pragma once
#include "Undo.h"
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <NodeRef.h>

namespace Glory::Editor
{
    class YAMLAction : public IAction
    {
    public:
        GLORY_EDITOR_API YAMLAction(YAMLFileRef& file, const std::filesystem::path& propertyPath, YAML::Node oldValue, YAML::Node newValue);

    private:
        virtual GLORY_EDITOR_API void OnUndo(const ActionRecord& actionRecord);
        virtual GLORY_EDITOR_API void OnRedo(const ActionRecord& actionRecord);

    private:
        friend class Undo;
        YAMLFileRef& m_File;
        const std::filesystem::path m_PropertyPath;
        YAML::Node m_OldValue;
        YAML::Node m_NewValue;
    };
}
