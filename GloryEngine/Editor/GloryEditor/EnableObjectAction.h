#pragma once
#include "Undo.h"
#include <GScene.h>

namespace Glory
{
    class GScene;
namespace Editor
{
    class EnableObjectAction : public IAction
    {
    public:
        GLORY_EDITOR_API EnableObjectAction(GScene* pScene, bool active);
        virtual ~EnableObjectAction();

    private:
        virtual void OnUndo(const ActionRecord& actionRecord);
        virtual void OnRedo(const ActionRecord& actionRecord);

    private:
        const UUID m_SceneID;
        const bool m_Active;
    };
}
}
