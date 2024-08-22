#pragma once
#include "Undo.h"
#include "GloryEditor.h"

#include "EntityID.h"
#include <NodeRef.h>

namespace Glory
{
    class GScene;
namespace Editor
{
    class CreateObjectAction : public IAction
    {
    public:
        GLORY_EDITOR_API CreateObjectAction(GScene* pScene);
        virtual GLORY_EDITOR_API ~CreateObjectAction();

    private:
        virtual void OnUndo(const ActionRecord& actionRecord);
        virtual void OnRedo(const ActionRecord& actionRecord);

    private:
        Utils::InMemoryYAML m_SerializedObject;
        UUID m_SceneID;
    };
}
}