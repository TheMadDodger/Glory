#include "CreateObjectAction.h"
#include "EditorSceneManager.h"
#include "EditorSceneSerializer.h"
#include "Selection.h"
#include "EditableEntity.h"
#include "EntityEditor.h"
#include "EditorApplication.h"

#include <GScene.h>

namespace Glory::Editor
{
	CreateObjectAction::CreateObjectAction(GScene* pScene) : m_SceneID(pScene->GetUUID())
	{
	}

	CreateObjectAction::~CreateObjectAction()
	{
	}

	void CreateObjectAction::OnUndo(const ActionRecord& actionRecord)
	{
		Selection::SetActiveObject(nullptr);

		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_SceneID);

		Entity entity = pScene->GetEntityByUUID(actionRecord.ObjectID);
		if (!entity.IsValid()) return;

		/* Take a snapshot of the object for redoing */
		auto entities = m_SerializedObject.RootNodeRef().ValueRef();
		entities.SetSequence();
		EditorSceneSerializer::SerializeEntityRecursive(EditorApplication::GetInstance()->GetEngine(), pScene, entity.GetEntityID(), entities);

		pScene->DestroyEntity(entity.GetEntityID());
	}

	void CreateObjectAction::OnRedo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_SceneID);
		if (pScene == nullptr) return;
		Utils::NodeRef entities = m_SerializedObject.RootNodeRef();
		for (size_t i = 0; i < entities.ValueRef().Size(); i++)
		{
			Utils::NodeValueRef entity = entities.ValueRef()[i];
			EditorSceneSerializer::DeserializeEntity(EditorApplication::GetInstance()->GetEngine(), pScene, entity);
		}

		Selection::SetActiveObject(GetEditableEntity(pScene->GetEntityByUUID(actionRecord.ObjectID).GetEntityID(), pScene));
	}
}