#include "SetParentAction.h"
#include "EditorSceneManager.h"
#include "EditorApplication.h"

#include <GScene.h>

namespace Glory::Editor
{
	SetParentAction::SetParentAction(GScene* pScene, UUID oldParent, UUID newParent, size_t siblingIndex)
		: m_SceneID(pScene->GetUUID()), m_OldParent(oldParent), m_NewParent(newParent), m_SiblingIndex(siblingIndex) {}

	void SetParentAction::OnUndo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_SceneID);
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(actionRecord.ObjectID);
		entity.SetParent(m_OldParent ? pScene->GetEntityByUUID(m_OldParent).GetEntityID() : 0);
		entity.SetSiblingIndex(m_SiblingIndex);
	}

	void SetParentAction::OnRedo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_SceneID);
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(actionRecord.ObjectID);
		entity.SetParent(m_NewParent ? pScene->GetEntityByUUID(m_NewParent).GetEntityID() : 0);
	}
}
