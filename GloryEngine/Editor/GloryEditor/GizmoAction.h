#pragma once
#include "Undo.h"
#include <UUID.h>
#include <glm/glm.hpp>
#include "GloryEditor.h"

namespace Glory::Editor
{
    class GizmoAction : public IAction
    {
    public:
        GLORY_EDITOR_API GizmoAction(UUID uuid, const glm::mat4& oldTransform, const glm::mat4& newTransform);
        virtual GLORY_EDITOR_API ~GizmoAction();

    private:
        virtual void OnUndo(const ActionRecord& actionRecord);
        virtual void OnRedo(const ActionRecord& actionRecord);

    private:
        const UUID m_GizmoUUID;
        const glm::mat4 m_OldTransform;
        const glm::mat4 m_NewTransform;
    };
}
