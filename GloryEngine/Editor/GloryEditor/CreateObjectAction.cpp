#include "CreateObjectAction.h"
#include <GScene.h>
#include <Game.h>
#include <Engine.h>
#include <ScenesModule.h>

namespace Glory::Editor
{
	CreateObjectAction::CreateObjectAction(SceneObject* pSceneObject) : m_SceneID(pSceneObject->GetScene()->GetUUID())
	{
		
	}

	CreateObjectAction::~CreateObjectAction()
	{
	}

	void CreateObjectAction::OnUndo(const ActionRecord& actionRecord)
	{
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		GScene* pScene = pScenesModule->GetOpenScene(m_SceneID);
		if (pScene == nullptr) return;
		SceneObject* pObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
		if (pObject == nullptr) return;
		pScene->DeleteObject(pObject);
	}

	void CreateObjectAction::OnRedo(const ActionRecord& actionRecord)
	{
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		GScene* pScene = pScenesModule->GetOpenScene(m_SceneID);
		if (pScene == nullptr) return;
		pScene->CreateEmptyObject("Empty Object", actionRecord.ObjectID);
	}
}