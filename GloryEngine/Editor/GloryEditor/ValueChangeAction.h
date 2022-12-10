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
        GLORY_EDITOR_API ValueChangeAction(const std::vector<const GloryReflect::FieldData*>& pFieldStack);
        GLORY_EDITOR_API virtual ~ValueChangeAction();

        GLORY_EDITOR_API void SetOldValue(void* pObject);
        GLORY_EDITOR_API void SetNewValue(void* pObject);

    private:
        virtual GLORY_EDITOR_API void OnUndo(const ActionRecord& actionRecord);
        virtual GLORY_EDITOR_API void OnRedo(const ActionRecord& actionRecord);

    private:
        const std::vector<const GloryReflect::FieldData*> m_pFieldStack;
        YAML::Node m_OldValue;
        YAML::Node m_NewValue;
    };
}
