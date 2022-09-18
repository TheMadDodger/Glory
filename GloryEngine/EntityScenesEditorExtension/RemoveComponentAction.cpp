#include "RemoveComponentAction.h"
#include <ScenesModule.h>
#include <Engine.h>
#include <Serializer.h>
#include <EntitySceneObjectSerializer.h>
#include <Editor.h>

namespace Glory::Editor
{
	RemoveComponentAction::RemoveComponentAction(EntityScene* pScene, EntityComponentData* pDeletedComponent, size_t componentIndex) : m_ComponentIndex(componentIndex)
	{
		YAML::Emitter out;
		EntitySceneObjectSerializer::SerializeComponent(pScene->GetRegistry(), pDeletedComponent, out);
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
		EntitySceneObject* pEntityObject = (EntitySceneObject*)editors[0]->GetTarget();
		EntityScene* pEntityScene = (EntityScene*)pEntityObject->GetScene();
		EntitySceneObjectSerializer::DeserializeComponent(pEntityScene, pEntityObject, m_ComponentIndex, node);

		for (size_t i = 0; i < editors.size(); i++)
		{
			editors[i]->Initialize();
		}
	}

	void RemoveComponentAction::OnRedo(const ActionRecord& actionRecord)
	{
		std::vector<Editor*> editors = Editor::FindEditors(actionRecord.ObjectID);
		if (!editors.size()) return;

		EntitySceneObject* pEntityObject = (EntitySceneObject*)editors[0]->GetTarget();
		EntityScene* pEntityScene = (EntityScene*)pEntityObject->GetScene();
		pEntityScene->GetRegistry()->RemoveComponent(pEntityObject->GetEntityHandle().GetEntityID(), m_ComponentIndex);

		for (size_t i = 0; i < editors.size(); i++)
		{
			editors[i]->Initialize();
		}
	}
}
