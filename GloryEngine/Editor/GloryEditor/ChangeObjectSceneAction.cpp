#include "ChangeObjectSceneAction.h"
#include <Engine.h>
#include <Game.h>
#include <SceneManager.h>

namespace Glory::Editor
{
	ChangeObjectSceneAction::ChangeObjectSceneAction(GScene* pOriginalScene, GScene* pNewScene)
		: m_OriginalSceneID(pOriginalScene->GetUUID()), m_NewSceneID(pNewScene->GetUUID())
	{
	}

	ChangeObjectSceneAction::~ChangeObjectSceneAction()
	{
	}

	void ChangeObjectSceneAction::OnUndo(const ActionRecord& actionRecord)
	{
		SceneObject* pObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
		if (pObject == nullptr) return;
		SceneManager* pSceneModule = Game::GetGame().GetEngine()->GetSceneManager();
		GScene* pOriginalScene = pSceneModule->GetOpenScene(m_OriginalSceneID);
		pObject->SetScene(pOriginalScene);
	}

	void ChangeObjectSceneAction::OnRedo(const ActionRecord& actionRecord)
	{
		SceneObject* pObject = (SceneObject*)Object::FindObject(actionRecord.ObjectID);
		if (pObject == nullptr) return;
		SceneManager* pSceneModule = Game::GetGame().GetEngine()->GetSceneManager();
		GScene* pNewScene = pSceneModule->GetOpenScene(m_NewSceneID);
		pObject->SetScene(pNewScene);
	}
}