#include "RemoveComponentAction.h"
#include "EditorSceneSerializer.h"
#include "EditableEntity.h"
#include "EditorSceneManager.h"
#include "Editor.h"
#include "EditorApplication.h"

#include <SceneManager.h>
#include <Engine.h>
#include <UUIDRemapper.h>

#include <EntityRegistry.h>

namespace Glory::Editor
{
	RemoveComponentAction::RemoveComponentAction(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entityID, size_t componentIndex) : m_ComponentIndex(componentIndex)
	{
		YAML::Emitter out;
		Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entityID);
		EditorSceneSerializer::SerializeComponent(EditorApplication::GetInstance()->GetEngine(), pRegistry, pEntityView, entityID, componentIndex, out);
		m_SerializedComponent = out.c_str();
	}

	RemoveComponentAction::~RemoveComponentAction()
	{
	}

	void RemoveComponentAction::OnUndo(const ActionRecord& actionRecord)
	{
		std::vector<Editor*> editors = Editor::FindEditors(actionRecord.ObjectID);
		if (!editors.size()) return;

		YAML::Node node = YAML::Load(m_SerializedComponent.c_str());
		EditableEntity* pEntityObject = (EditableEntity*)editors[0]->GetTarget();
		GScene* pEntityScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(pEntityObject->SceneID());
		Utils::ECS::EntityView* pEntityView = pEntityScene->GetRegistry().GetEntityView(pEntityObject->EntityID());
		const size_t index = pEntityView->ComponentCount();
		EditorSceneSerializer::DeserializeComponent(EditorApplication::GetInstance()->GetEngine(), pEntityScene, pEntityObject->EntityID(), UUIDRemapper{}, node);
		pEntityView->SetComponentIndex(index, m_ComponentIndex);

		for (size_t i = 0; i < editors.size(); i++)
		{
			editors[i]->Initialize();
		}
	}

	void RemoveComponentAction::OnRedo(const ActionRecord& actionRecord)
	{
		std::vector<Editor*> editors = Editor::FindEditors(actionRecord.ObjectID);
		if (!editors.size()) return;

		EditableEntity* pEntityObject = (EditableEntity*)editors[0]->GetTarget();
		GScene* pEntityScene = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(pEntityObject->SceneID());
		pEntityScene->GetRegistry().RemoveComponentAt(pEntityObject->EntityID(), m_ComponentIndex);

		for (size_t i = 0; i < editors.size(); i++)
		{
			editors[i]->Initialize();
		}
	}
}
