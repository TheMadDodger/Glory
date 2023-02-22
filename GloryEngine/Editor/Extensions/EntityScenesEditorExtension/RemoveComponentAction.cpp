#include "RemoveComponentAction.h"
#include <ScenesModule.h>
#include <Engine.h>
#include <Serializer.h>
#include <EntitySceneObjectSerializer.h>
#include <Editor.h>

namespace Glory::Editor
{
	RemoveComponentAction::RemoveComponentAction(EntityRegistry* pRegistry, EntityID entityID, size_t componentIndex) : m_ComponentIndex(componentIndex)
	{
		YAML::Emitter out;
		EntityView* pEntityView = pRegistry->GetEntityView(entityID);
		uint32_t typeHash = pEntityView->ComponentTypeAt(componentIndex);
		UUID componentUUID = pEntityView->ComponentUUIDAt(componentIndex);
		void* pAddress = pRegistry->GetComponentAddress(entityID, componentUUID);
		const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
		EntitySceneObjectSerializer::SerializeComponent(entityID, pRegistry, componentUUID, pAddress, pTypeData, out);
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
		pEntityScene->GetRegistry()->RemoveComponentAt(pEntityObject->GetEntityHandle().GetEntityID(), m_ComponentIndex);

		for (size_t i = 0; i < editors.size(); i++)
		{
			editors[i]->Initialize();
		}
	}
}
