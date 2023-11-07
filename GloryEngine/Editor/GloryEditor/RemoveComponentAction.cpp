#include "RemoveComponentAction.h"

#include <SceneManager.h>
#include <Engine.h>
#include <Serializer.h>
#include <SceneObjectSerializer.h>
#include <EntityRegistry.h>
#include <Editor.h>

namespace Glory::Editor
{
	RemoveComponentAction::RemoveComponentAction(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entityID, size_t componentIndex) : m_ComponentIndex(componentIndex)
	{
		YAML::Emitter out;
		Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entityID);
		uint32_t typeHash = pEntityView->ComponentTypeAt(componentIndex);
		UUID componentUUID = pEntityView->ComponentUUIDAt(componentIndex);
		void* pAddress = pRegistry->GetComponentAddress(entityID, componentUUID);
		const TypeData* pTypeData = Reflect::GetTyeData(typeHash);
		SceneObjectSerializer::SerializeComponent(entityID, pRegistry, componentUUID, pAddress, pTypeData, out);
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
		SceneObject* pEntityObject = (SceneObject*)editors[0]->GetTarget();
		GScene* pEntityScene = pEntityObject->GetScene();
		SceneObjectSerializer::DeserializeComponent(pEntityScene, pEntityObject, m_ComponentIndex, node);

		for (size_t i = 0; i < editors.size(); i++)
		{
			editors[i]->Initialize();
		}
	}

	void RemoveComponentAction::OnRedo(const ActionRecord& actionRecord)
	{
		std::vector<Editor*> editors = Editor::FindEditors(actionRecord.ObjectID);
		if (!editors.size()) return;

		SceneObject* pEntityObject = (SceneObject*)editors[0]->GetTarget();
		GScene* pEntityScene = pEntityObject->GetScene();
		pEntityScene->GetRegistry()->RemoveComponentAt(pEntityObject->GetEntityHandle().GetEntityID(), m_ComponentIndex);

		for (size_t i = 0; i < editors.size(); i++)
		{
			editors[i]->Initialize();
		}
	}
}
