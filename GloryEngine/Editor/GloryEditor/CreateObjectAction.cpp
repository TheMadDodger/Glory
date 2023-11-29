#include "CreateObjectAction.h"
#include "EditorSceneManager.h"
#include "SceneSerializer.h"
#include "Selection.h"
#include "EditableEntity.h"
#include "EntityEditor.h"

#include <GScene.h>
#include <Game.h>

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

		GScene* pScene = EditorSceneManager::GetOpenScene(m_SceneID);

		Entity entity = pScene->GetEntityByUUID(actionRecord.ObjectID);
		if (!entity.IsValid()) return;

		/* Take a snapshot of the object for redoing */
		YAML::Emitter out;
		out << YAML::BeginSeq;
		SceneSerializer::SerializeEntityRecursive(pScene, entity.GetEntityID(), out);
		out << YAML::EndSeq;
		m_SerializedObject = out.c_str();

		pScene->DestroyEntity(entity.GetEntityID());
	}

	void CreateObjectAction::OnRedo(const ActionRecord& actionRecord)
	{
		GScene* pScene = EditorSceneManager::GetOpenScene(m_SceneID);
		if (pScene == nullptr) return;
		YAML::Node node = YAML::Load(m_SerializedObject.c_str());
		Utils::NodeRef entities{node};
		for (size_t i = 0; i < entities.ValueRef().Size(); i++)
		{
			Utils::NodeValueRef entity = entities.ValueRef()[i];
			SceneSerializer::DeserializeEntity(pScene, entity.Node());
		}

		Selection::SetActiveObject(GetEditableEntity(pScene->GetEntityByUUID(actionRecord.ObjectID).GetEntityID(), pScene));
	}
}