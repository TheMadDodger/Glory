#include "AddComponentAction.h"

namespace Glory::Editor
{
	AddComponentAction::AddComponentAction(size_t typeHash, UUID uuid, size_t componentIndex) : m_ComponentTypeHash(typeHash), m_ComponentUUID(uuid), m_ComponentIndex(componentIndex)
	{
	}

	AddComponentAction::~AddComponentAction()
	{
	}

	void AddComponentAction::OnUndo(const ActionRecord& actionRecord)
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

	void AddComponentAction::OnRedo(const ActionRecord& actionRecord)
	{
		std::vector<Editor*> editors = Editor::FindEditors(actionRecord.ObjectID);
		if (!editors.size()) return;

		EntitySceneObject* pEntityObject = (EntitySceneObject*)editors[0]->GetTarget();
		EntityScene* pEntityScene = (EntityScene*)pEntityObject->GetScene();
		GloryECS::EntityRegistry* pRegistry = pEntityScene->GetRegistry();
		EntityID entity = pEntityObject->GetEntityHandle().GetEntityID();
		pRegistry->CreateComponent(entity, m_ComponentTypeHash, m_ComponentUUID);
		EntityView* pEntityView = pRegistry->GetEntityView(entity);
		pEntityView->SetComponentIndex(pEntityView->ComponentCount(), m_ComponentIndex);

		for (size_t i = 0; i < editors.size(); i++)
		{
			editors[i]->Initialize();
		}
	}
}
