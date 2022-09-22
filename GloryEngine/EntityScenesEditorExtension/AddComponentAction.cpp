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
		pEntityScene->GetRegistry()->RemoveComponent(pEntityObject->GetEntityHandle().GetEntityID(), m_ComponentIndex);

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
		EntitySystems* pSystems = pEntityScene->GetRegistry()->GetSystems();
		EntityID entity = pEntityObject->GetEntityHandle().GetEntityID();
		pSystems->CreateComponent(entity, m_ComponentTypeHash, m_ComponentUUID);

		for (size_t i = 0; i < editors.size(); i++)
		{
			editors[i]->Initialize();
		}
	}
}
