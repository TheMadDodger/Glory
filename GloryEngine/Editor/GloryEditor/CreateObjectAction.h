#pragma once
#include "Undo.h"
#include "GloryEditor.h"
#include <SceneObject.h>
#include <yaml-cpp/yaml.h>

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

        void SerializeRecursive(SceneObject* pObject, YAML::Emitter& out);

    private:
        std::string m_SerializedObject;
        UUID m_SceneID;
    };
}
