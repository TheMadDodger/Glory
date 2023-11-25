#pragma once
#include "Undo.h"
#include "GloryEditor.h"

#include "EntityID.h"
#include <yaml-cpp/yaml.h>

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
        std::string m_SerializedObject;
        UUID m_SceneID;
    };
}
}