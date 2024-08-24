#pragma once
#include "Undo.h"
#include "Object.h"
#include "GloryEditor.h"

namespace Glory::Editor
{
    class SelectionChangedAction : public IAction
    {
    public:
        GLORY_EDITOR_API SelectionChangedAction(std::vector<UUID>&& oldSelection);
        virtual GLORY_EDITOR_API ~SelectionChangedAction();

    private:
        virtual void OnUndo(const ActionRecord& actionRecord);
        virtual void OnRedo(const ActionRecord& actionRecord);

    private:
        std::vector<UUID> m_OldSelection;
        std::vector<UUID> m_NewSelection;
    };
}
