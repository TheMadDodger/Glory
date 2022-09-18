#include "DeleteSceneObjectAction.h"
#include <ScenesModule.h>
#include <Engine.h>
#include <Serializer.h>

namespace Glory::Editor
{
	DeleteSceneObjectAction::DeleteSceneObjectAction(SceneObject* pDeletedObject) : m_OriginalSceneUUID(pDeletedObject->GetScene()->GetUUID())
	{
		YAML::Emitter out;
		Serializer::SerializeObject(pDeletedObject, out);
		m_SerializedObject = out.c_str();
	}

	DeleteSceneObjectAction::~DeleteSceneObjectAction()
	{
	}

	void DeleteSceneObjectAction::OnUndo(const ActionRecord& actionRecord)
	{
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		GScene* pScene = pScenesModule->GetOpenScene(m_OriginalSceneUUID);
		if (pScene == nullptr) return;
		YAML::Node node = YAML::Load(m_SerializedObject.c_str());
		SceneObject* pSceneObject = (SceneObject*)Serializer::DeserializeObject(pScene, node);
	}

	void DeleteSceneObjectAction::OnRedo(const ActionRecord& actionRecord)
	{
		SceneObject* pSceneObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
		if (pSceneObject == nullptr) return;
		pSceneObject->GetScene()->DeleteObject(pSceneObject);
	}
}
