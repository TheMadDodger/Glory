#include "GizmoAction.h"
#include "Gizmos.h"

namespace Glory::Editor
{
	GizmoAction::GizmoAction(UUID uuid, const glm::mat4& oldTransform, const glm::mat4& newTransform) : m_GizmoUUID(uuid), m_OldTransform(oldTransform), m_NewTransform(newTransform)
	{
	}

	GizmoAction::~GizmoAction()
	{
	}

	void GizmoAction::OnUndo(const ActionRecord& actionRecord)
	{
		IGizmo* pGizmo = Gizmos::FindGizmo(m_GizmoUUID);
		if (pGizmo == nullptr) return; // Should not be possible but eeeeeeeeeeeeh
		pGizmo->ManualManipulate(m_OldTransform);
	}

	void GizmoAction::OnRedo(const ActionRecord& actionRecord)
	{
		IGizmo* pGizmo = Gizmos::FindGizmo(m_GizmoUUID);
		if (pGizmo == nullptr) return; // Should not be possible but eeeeeeeeeeeeh
		pGizmo->ManualManipulate(m_NewTransform);
	}
}
