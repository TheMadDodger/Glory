#include "SetSiblingIndexAction.h"
#include "EditorSceneManager.h"

#include <GScene.h>

namespace Glory::Editor
{
	SetSiblingIndexAction::SetSiblingIndexAction(GScene* pScene, size_t oldSiblingIndex, size_t newSiblingIndex)
		: m_SceneID(pScene->GetUUID()), m_OldSiblingIndex(oldSiblingIndex), m_NewSiblingIndex(newSiblingIndex)
	{
	}

	SetSiblingIndexAction::~SetSiblingIndexAction()
	{
	}

	void SetSiblingIndexAction::OnUndo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorSceneManager::GetOpenScene(m_SceneID);
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(actionRecord.ObjectID);
		if (!entity.IsValid()) return;

		size_t index = m_OldSiblingIndex;
		if (index > m_NewSiblingIndex)
			++index;
		entity.SetSiblingIndex(index);
	}

	void SetSiblingIndexAction::OnRedo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorSceneManager::GetOpenScene(m_SceneID);
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(actionRecord.ObjectID);
		if (!entity.IsValid()) return;

		size_t index = m_NewSiblingIndex;
		if (index > m_OldSiblingIndex)
			++index;
		entity.SetSiblingIndex(m_NewSiblingIndex);
	}
}
