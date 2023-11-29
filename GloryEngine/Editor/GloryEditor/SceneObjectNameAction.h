#pragma once
#include "Undo.h"
#include "GloryEditor.h"

namespace Glory
{
    class GScene;
namespace Editor
{
    class SceneObjectNameAction : public IAction
    {
    public:
        GLORY_EDITOR_API SceneObjectNameAction(GScene* pScene, const std::string_view original, const std::string_view name);
        virtual GLORY_EDITOR_API ~SceneObjectNameAction();

    private:
        virtual void OnUndo(const ActionRecord& actionRecord);
        virtual void OnRedo(const ActionRecord& actionRecord);

    private:
        const UUID m_SceneID;
        const std::string m_Original;
        const std::string m_NewName;
    };
}
}
