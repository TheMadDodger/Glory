//#include "SetParentAction.h"
//
//#include <SceneObject.h>
//
//namespace Glory::Editor
//{
//	SetParentAction::SetParentAction(UUID oldParent, UUID newParent, size_t siblingIndex)
//		: m_OldParent(oldParent), m_NewParent(newParent), m_SiblingIndex(siblingIndex) {}
//
//	void SetParentAction::OnUndo(const ActionRecord& actionRecord)
//	{
//		SceneObject* pObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
//		if (!pObject) return;
//		SceneObject* pParent = (SceneObject*)Object::FindObject(m_OldParent);
//		pObject->SetParent(pParent);
//		pObject->SetSiblingIndex(m_SiblingIndex);
//	}
//
//	void SetParentAction::OnRedo(const ActionRecord& actionRecord)
//	{
//		SceneObject* pObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
//		if (!pObject) return;
//		SceneObject* pParent = (SceneObject*)Object::FindObject(m_NewParent);
//		pObject->SetParent(pParent);
//	}
//}
