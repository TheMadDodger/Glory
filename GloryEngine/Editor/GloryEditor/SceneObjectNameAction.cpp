#include "SceneObjectNameAction.h"
#include "EditorSceneManager.h"

#include <GScene.h>

namespace Glory::Editor
{
	SceneObjectNameAction::SceneObjectNameAction(GScene* pScene, const std::string_view original, const std::string_view name)
		: m_SceneID(pScene->GetUUID()), m_Original(original), m_NewName(name)
	{
	}

	SceneObjectNameAction::~SceneObjectNameAction()
	{
	}

	void SceneObjectNameAction::OnUndo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorSceneManager::GetOpenScene(m_SceneID);
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(actionRecord.ObjectID);
		pScene->SetEntityName(entity.GetEntityID(), m_Original);
	}

	void SceneObjectNameAction::OnRedo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorSceneManager::GetOpenScene(m_SceneID);
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(actionRecord.ObjectID);
		pScene->SetEntityName(entity.GetEntityID(), m_NewName);
	}
}
