//#include "CreateObjectAction.h"
//#include <GScene.h>
//#include <Game.h>
//#include <Engine.h>
//#include <SceneManager.h>
//#include <Serializer.h>
//
//namespace Glory::Editor
//{
//	CreateObjectAction::CreateObjectAction(SceneObject* pSceneObject) : m_SceneID(pSceneObject->GetScene()->GetUUID())
//	{
//	}
//
//	CreateObjectAction::~CreateObjectAction()
//	{
//	}
//
//	void CreateObjectAction::OnUndo(const ActionRecord& actionRecord)
//	{
//		SceneObject* pSceneObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
//		if (pSceneObject == nullptr) return;
//
//		/* Take a snapshot of the object for redoing */
//		YAML::Emitter out;
//		out << YAML::BeginSeq;
//		SerializeRecursive(pSceneObject, out);
//		out << YAML::EndSeq;
//		m_SerializedObject = out.c_str();
//
//		pSceneObject->GetScene()->DeleteObject(pSceneObject);
//	}
//
//	void CreateObjectAction::OnRedo(const ActionRecord& actionRecord)
//	{
//		SceneManager* pScenesModule = Game::GetGame().GetEngine()->GetSceneManager();
//		GScene* pScene = pScenesModule->GetOpenScene(m_SceneID);
//		if (pScene == nullptr) return;
//		YAML::Node node = YAML::Load(m_SerializedObject.c_str());
//		for (size_t i = 0; i < node.size(); i++)
//		{
//			YAML::Node subNode = node[i];
//			SceneObject* pSceneObject = (SceneObject*)Serializer::DeserializeObject(pScene, subNode);
//		}
//	}
//
//	void CreateObjectAction::SerializeRecursive(SceneObject* pObject, YAML::Emitter& out)
//	{
//		Serializer::SerializeObject(pObject, out);
//		for (size_t i = 0; i < pObject->ChildCount(); i++)
//		{
//			SceneObject* pChild = pObject->GetChild(i);
//			SerializeRecursive(pChild, out);
//		}
//	}
//}