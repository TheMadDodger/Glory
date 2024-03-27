#include "EnableObjectAction.h"
#include "EditorSceneManager.h"
#include "EditorApplication.h"

#include <GScene.h>

namespace Glory::Editor
{
	EnableObjectAction::EnableObjectAction(GScene* pScene, bool active)
		: m_SceneID(pScene->GetUUID()), m_Active(active)
	{
	}

	EnableObjectAction::~EnableObjectAction()
	{
	}

	void EnableObjectAction::OnUndo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_SceneID);
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(actionRecord.ObjectID);
		entity.SetActive(!m_Active);
	}

	void EnableObjectAction::OnRedo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_SceneID);
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(actionRecord.ObjectID);
		entity.SetActive(m_Active);
	}
}