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
	DeleteSceneObjectAction::DeleteSceneObjectAction(GScene* pScene, Utils::ECS::EntityID deletedEntity) : m_OriginalSceneUUID(pScene->GetUUID()), m_WasSelected(false)
	{
		Object* pActiveObject = Selection::GetActiveObject();
		if (pActiveObject != nullptr)
		{
			EditableEntity* pEntity = GetEditableEntity(deletedEntity, pScene);
			m_WasSelected = pEntity != nullptr && pActiveObject == pEntity;
		}

		auto entities = m_SerializedObject.RootNodeRef().ValueRef();
		entities.SetSequence();
		EditorSceneSerializer::SerializeEntityRecursive(EditorApplication::GetInstance()->GetEngine(), pScene, deletedEntity, entities);
	}

	DeleteSceneObjectAction::~DeleteSceneObjectAction()
	{
	}

	void GatherChildrenRecursive(const Utils::ECS::EntityID parent, Utils::ECS::EntityRegistry* pRegistry, std::vector<Utils::ECS::EntityID>& children)
	{
		for (size_t i = 0; i < pRegistry->ChildCount(parent); ++i)
		{
			const Utils::ECS::EntityID child = pRegistry->Child(parent, i);
			children.push_back(child);
			GatherChildrenRecursive(child, pRegistry, children);
		}
	}

	void DeleteSceneObjectAction::OnUndo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(m_OriginalSceneUUID);
		if (pScene == nullptr) return;
		auto entities = m_SerializedObject.RootNodeRef().ValueRef();
		std::vector<Utils::ECS::EntityID> newEntities;
		for (size_t i = 0; i < entities.Size(); i++)
		{
			Utils::NodeValueRef entity = entities[i];
			Entity newEntity = EditorSceneSerializer::DeserializeEntity(EditorApplication::GetInstance()->GetEngine(), pScene, entity);

			newEntities.push_back(newEntity.GetEntityID());
			GatherChildrenRecursive(newEntity.GetEntityID(), newEntity.GetRegistry(), newEntities);
		}

		pScene->GetRegistry().InvokeAll(Utils::ECS::InvocationType::OnEnableDraw, newEntities);

		if (!m_WasSelected) return;
		Selection::SetActiveObject(GetEditableEntity(pScene->GetEntityByUUID(actionRecord.ObjectID).GetEntityID(), pScene));
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

		DestroyEntity(entity.GetEntityID(), pScene);
	}
}
