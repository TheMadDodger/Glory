#pragma once
#include "Undo.h"
#include "Object.h"
#include "GloryEditor.h"

namespace Glory::Editor
{
    class SelectAction : public IAction
    {
    public:
        GLORY_EDITOR_API SelectAction(UUID selectedObjectID);
        virtual GLORY_EDITOR_API ~SelectAction();

    private:
        virtual void OnUndo(const ActionRecord& actionRecord);
        virtual void OnRedo(const ActionRecord& actionRecord);

    private:
        UUID m_SelectedObjectID;
    };
}
