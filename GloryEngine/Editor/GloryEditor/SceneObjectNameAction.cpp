//#include "SceneObjectNameAction.h"
//#include <SceneObject.h>
//
//namespace Glory::Editor
//{
//	SceneObjectNameAction::SceneObjectNameAction(const std::string& original, const std::string& name) : m_Original(original), m_NewName(name)
//	{
//	}
//
//	SceneObjectNameAction::~SceneObjectNameAction()
//	{
//	}
//
//	void SceneObjectNameAction::OnUndo(const ActionRecord& actionRecord)
//	{
//		SceneObject* pObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
//		pObject->SetName(m_Original);
//	}
//
//	void SceneObjectNameAction::OnRedo(const ActionRecord& actionRecord)
//	{
//		SceneObject* pObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
//		pObject->SetName(m_NewName);
//	}
//}
