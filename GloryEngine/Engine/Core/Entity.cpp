#include "Entity.h"
#include "GScene.h"

namespace Glory
{
	Entity::Entity() : m_EntityID(NULL), m_pGScene(nullptr), m_pRegistry(nullptr)
	{
	}

	Entity::Entity(Utils::ECS::EntityID entityHandle, GScene* pScene) :
		m_EntityID(entityHandle), m_pGScene(pScene), m_pRegistry(pScene->GetRegistry())
	{
	}

	Utils::ECS::EntityView* Entity::GetEntityView()
	{
		return m_pRegistry->GetEntityView(m_EntityID);
	}

	void Entity::Clear()
	{
		m_pRegistry->Clear(m_EntityID);
	}

	bool Entity::IsValid()
	{
		if (!m_pGScene) return false;
		return m_pRegistry->IsValid(m_EntityID);
	}

	//void Entity::ForEachComponent(std::function<void(Registry*, EntityID, EntityComponentData*)> func)
	//{
	//	m_pGScene->m_Registry.ForEachComponent(m_EntityID, func);
	//}

	void Entity::Destroy()
	{
		if (!m_pGScene->IsValid()) return;
		m_pRegistry->DestroyEntity(m_EntityID);
	}

	Utils::ECS::EntityID Entity::GetEntityID()
	{
		return m_EntityID;
	}

	Utils::ECS::EntityRegistry* Entity::GetRegistry()
	{
		return m_pRegistry;
	}

	GScene* Entity::GetScene()
	{
		return m_pGScene;
	}

	bool Entity::IsActiveSelf() const
	{
		return m_pRegistry->GetEntityView(m_EntityID)->Active();
	}

	bool Entity::IsActive() const
	{
		return m_pRegistry->GetEntityView(m_EntityID)->IsActive();
	}

	void Entity::SetActive(bool active)
	{
		m_pRegistry->GetEntityView(m_EntityID)->Active() = active;
	}

	void Entity::SetActiveHierarchy(bool active)
	{
		m_pRegistry->GetEntityView(m_EntityID)->HierarchyActive() = active;
	}
}