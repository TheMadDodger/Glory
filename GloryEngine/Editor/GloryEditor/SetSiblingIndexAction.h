#pragma once
#include "Undo.h"

namespace Glory
{
    class GScene;
namespace Editor
{
    class SetSiblingIndexAction : public IAction
    {
    public:
        GLORY_EDITOR_API SetSiblingIndexAction(GScene* pScene, size_t oldSiblingIndex, size_t newSiblingIndex);
        virtual GLORY_EDITOR_API ~SetSiblingIndexAction();

    private:
        virtual void OnUndo(const ActionRecord& actionRecord);
        virtual void OnRedo(const ActionRecord& actionRecord);

    private:
        const UUID m_SceneID;
        const size_t m_OldSiblingIndex;
        const size_t m_NewSiblingIndex;
    };
}
}
