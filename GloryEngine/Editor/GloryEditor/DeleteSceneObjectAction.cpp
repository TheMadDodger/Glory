#include "DeleteSceneObjectAction.h"
#include "EditorSceneManager.h"
#include "Selection.h"
#include "EntityEditor.h"
#include "EditableEntity.h"
#include "EditorSceneSerializer.h"
#include "EditorApplication.h"

#include <GScene.h>

namespace Glory::Editor
{
	DeleteSceneObjectAction::DeleteSceneObjectAction(GScene* pScene, Utils::ECS::EntityID deletedEntity) : m_OriginalSceneUUID(pScene->GetUUID())
	{
		auto entities = m_SerializedObject.RootNodeRef().ValueRef();
		entities.SetSequence();
		EditorSceneSerializer::SerializeEntityRecursive(EditorApplication::GetInstance()->GetEngine(), pScene, deletedEntity, entities);
	}

	DeleteSceneObjectAction::~DeleteSceneObjectAction()
	{
	}

	void DeleteSceneObjectAction::OnUndo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_OriginalSceneUUID);
		if (pScene == nullptr) return;
		auto entities = m_SerializedObject.RootNodeRef().ValueRef();
		for (size_t i = 0; i < entities.Size(); i++)
		{
			Utils::NodeValueRef entity = entities[i];
			EditorSceneSerializer::DeserializeEntity(EditorApplication::GetInstance()->GetEngine(), pScene, entity);
		}

		if (!m_WasSelected) return;
		Selection::SetActiveObject(GetEditableEntity(pScene->GetEntityByUUID(actionRecord.ObjectID).EntityUUID(), pScene));
	}

	void DeleteSceneObjectAction::OnRedo(const ActionRecord& actionRecord)
	{
		m_WasSelected = false;
		if (Selection::GetActiveObject()->GetUUID() == actionRecord.ObjectID)
		{
			Selection::SetActiveObject(nullptr);
			m_WasSelected = true;
		}

		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_OriginalSceneUUID);

		Entity entity = pScene->GetEntityByUUID(actionRecord.ObjectID);
		if (!entity.IsValid()) return;

		/*
		 * Take a snapshot of the object for redoing
		 * Maybe not needed?
		 */
		//auto entities = m_SerializedObject.RootNodeRef().ValueRef();
		//entities.SetSequence();
		//EditorSceneSerializer::SerializeEntityRecursive(EditorApplication::GetInstance()->GetEngine(), pScene, entity.GetEntityID(), entities);

		pScene->DestroyEntity(entity.GetEntityID());
	}
}
