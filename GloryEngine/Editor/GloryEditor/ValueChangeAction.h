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
        GLORY_EDITOR_API ValueChangeAction(const Utils::Reflect::TypeData* pType, const std::filesystem::path& path);
        GLORY_EDITOR_API virtual ~ValueChangeAction();

        GLORY_EDITOR_API void SetOldValue(void* pObject);
        GLORY_EDITOR_API void SetNewValue(void* pObject);

    private:
        virtual GLORY_EDITOR_API void OnUndo(const ActionRecord& actionRecord);
        virtual GLORY_EDITOR_API void OnRedo(const ActionRecord& actionRecord);
        virtual GLORY_EDITOR_API bool Combine(IAction* pOther);

    private:
        const Utils::Reflect::TypeData* m_pRootType;
        const std::filesystem::path m_PropertyPath;
        Utils::InMemoryYAML m_OldValue;
        Utils::InMemoryYAML m_NewValue;
    };
}
