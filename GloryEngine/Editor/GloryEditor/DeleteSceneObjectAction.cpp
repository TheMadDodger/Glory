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
		YAML::Emitter out;
		out << YAML::BeginSeq;
		EditorSceneSerializer::SerializeEntityRecursive(EditorApplication::GetInstance()->GetEngine(), pScene, deletedEntity, out);
		out << YAML::EndSeq;
		m_SerializedObject = out.c_str();
	}

	DeleteSceneObjectAction::~DeleteSceneObjectAction()
	{
	}

	void DeleteSceneObjectAction::OnUndo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorSceneManager::GetOpenScene(m_OriginalSceneUUID);
		if (pScene == nullptr) return;
		YAML::Node node = YAML::Load(m_SerializedObject.c_str());
		Utils::NodeRef entities{node};
		for (size_t i = 0; i < entities.ValueRef().Size(); i++)
		{
			Utils::NodeValueRef entity = entities.ValueRef()[i];
			EditorSceneSerializer::DeserializeEntity(EditorApplication::GetInstance()->GetEngine(), pScene, entity.Node());
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

		GScene* pScene = EditorSceneManager::GetOpenScene(m_OriginalSceneUUID);

		Entity entity = pScene->GetEntityByUUID(actionRecord.ObjectID);
		if (!entity.IsValid()) return;

		/* Take a snapshot of the object for redoing */
		YAML::Emitter out;
		out << YAML::BeginSeq;
		EditorSceneSerializer::SerializeEntityRecursive(EditorApplication::GetInstance()->GetEngine(), pScene, entity.GetEntityID(), out);
		out << YAML::EndSeq;
		m_SerializedObject = out.c_str();

		pScene->DestroyEntity(entity.GetEntityID());
	}
}
