#pragma once
#include "Undo.h"
#include <filesystem>
#include <TypeData.h>
#include <yaml-cpp/yaml.h>

namespace Glory::Editor
{
    class ValueChangeAction : public IAction
    {
    public:
        GLORY_EDITOR_API ValueChangeAction(const GloryReflect::TypeData* pType, const std::filesystem::path& path);
        GLORY_EDITOR_API virtual ~ValueChangeAction();

        GLORY_EDITOR_API void SetOldValue(void* pObject);
        GLORY_EDITOR_API void SetNewValue(void* pObject);

    private:
        virtual GLORY_EDITOR_API void OnUndo(const ActionRecord& actionRecord);
        virtual GLORY_EDITOR_API void OnRedo(const ActionRecord& actionRecord);

    private:
        const GloryReflect::TypeData* m_pRootType;
        const std::filesystem::path m_PropertyPath;
        YAML::Node m_OldValue;
        YAML::Node m_NewValue;
    };
}
