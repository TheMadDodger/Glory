//#include "DeleteSceneObjectAction.h"
//#include <SceneManager.h>
//#include <Serializer.h>
//#include <SceneObject.h>
//#include <GScene.h>
//#include <Engine.h>
//
//namespace Glory::Editor
//{
//	DeleteSceneObjectAction::DeleteSceneObjectAction(SceneObject* pDeletedObject) : m_OriginalSceneUUID(pDeletedObject->GetScene()->GetUUID())
//	{
//		YAML::Emitter out;
//		out << YAML::BeginSeq;
//		SerializeRecursive(pDeletedObject, out);
//		out << YAML::EndSeq;
//		m_SerializedObject = out.c_str();
//	}
//
//	DeleteSceneObjectAction::~DeleteSceneObjectAction()
//	{
//	}
//
//	void DeleteSceneObjectAction::OnUndo(const ActionRecord& actionRecord)
//	{
//		SceneManager* pScenesModule = Game::GetGame().GetEngine()->GetSceneManager();
//		GScene* pScene = pScenesModule->GetOpenScene(m_OriginalSceneUUID);
//		if (pScene == nullptr) return;
//		YAML::Node node = YAML::Load(m_SerializedObject.c_str());
//		for (size_t i = 0; i < node.size(); i++)
//		{
//			YAML::Node subNode = node[i];
//			SceneObject* pSceneObject = Serializer::DeserializeObject<SceneObject>(pScene, subNode);
//		}
//	}
//
//	void DeleteSceneObjectAction::OnRedo(const ActionRecord& actionRecord)
//	{
//		SceneObject* pSceneObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
//		if (pSceneObject == nullptr) return;
//		pSceneObject->GetScene()->DeleteObject(pSceneObject);
//	}
//
//	void DeleteSceneObjectAction::SerializeRecursive(SceneObject* pDeletedObject, YAML::Emitter& out)
//	{
//		Serializer::SerializeObject(pDeletedObject, out);
//		for (size_t i = 0; i < pDeletedObject->ChildCount(); i++)
//		{
//			SceneObject* pChild = pDeletedObject->GetChild(i);
//			SerializeRecursive(pChild, out);
//		}
//	}
//}
