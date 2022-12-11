#pragma once
#include "Undo.h"
#include <filesystem>
#include <TypeData.h>
#include <yaml-cpp/yaml.h>

namespace Glory::Editor
{
    class NodeValueChangeAction : public IAction
    {
    public:
        GLORY_EDITOR_API NodeValueChangeAction(const std::filesystem::path& propertyPath, YAML::Node oldValue, YAML::Node newValue);
        GLORY_EDITOR_API virtual ~NodeValueChangeAction();

    private:
        virtual GLORY_EDITOR_API void OnUndo(const ActionRecord& actionRecord);
        virtual GLORY_EDITOR_API void OnRedo(const ActionRecord& actionRecord);

        void Set(std::vector<std::filesystem::path>& splitPath, YAML::Node& node, YAML::Node& value);

    private:
        const std::filesystem::path m_PropertyPath;
        YAML::Node m_OldValue;
        YAML::Node m_NewValue;
    };
}
