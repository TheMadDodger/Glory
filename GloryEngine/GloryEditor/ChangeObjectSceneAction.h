#pragma once
#include "Undo.h"
#include <GScene.h>

namespace Glory::Editor
{
    class ChangeObjectSceneAction : public IAction
    {
    public:
        ChangeObjectSceneAction(GScene* pOriginalScene, GScene* pNewScene);
        virtual ~ChangeObjectSceneAction();

    private:
        virtual void OnUndo(const ActionRecord& actionRecord);
        virtual void OnRedo(const ActionRecord& actionRecord);

    private:
        UUID m_OriginalSceneID;
        UUID m_NewSceneID;
    };
}
