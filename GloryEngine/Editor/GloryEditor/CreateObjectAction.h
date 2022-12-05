#pragma once
#include "Undo.h"
#include <SceneObject.h>
#include "GloryEditor.h"

namespace Glory::Editor
{
    class CreateObjectAction : public IAction
    {
    public:
        GLORY_EDITOR_API CreateObjectAction(SceneObject* pSceneObject);
        virtual GLORY_EDITOR_API ~CreateObjectAction();

    private:
        virtual void OnUndo(const ActionRecord& actionRecord);
        virtual void OnRedo(const ActionRecord& actionRecord);

    private:
        UUID m_SceneID;
    };
}
