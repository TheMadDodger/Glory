#include "SetSiblingIndexAction.h"
#include <SceneObject.h>

namespace Glory::Editor
{
	SetSiblingIndexAction::SetSiblingIndexAction(size_t oldSiblingIndex, size_t newSiblingIndex)
		: m_OldSiblingIndex(oldSiblingIndex), m_NewSiblingIndex(newSiblingIndex)
	{
	}

	SetSiblingIndexAction::~SetSiblingIndexAction()
	{
	}

	void SetSiblingIndexAction::OnUndo(const ActionRecord& actionRecord)
	{
		SceneObject* pObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
		if (!pObject) return;
		pObject->SetSiblingIndex(m_OldSiblingIndex);
	}

	void SetSiblingIndexAction::OnRedo(const ActionRecord& actionRecord)
	{
		SceneObject* pObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
		if (!pObject) return;
		pObject->SetSiblingIndex(m_NewSiblingIndex);
	}
}
