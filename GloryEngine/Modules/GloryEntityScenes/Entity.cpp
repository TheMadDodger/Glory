#include "Entity.h"

namespace Glory
{
	Entity::Entity() : m_EntityID(NULL), m_pEntityScene(nullptr)
	{
	}

	Entity::Entity(EntityID entityHandle, EntityScene* pScene) : m_EntityID(entityHandle), m_pEntityScene(pScene)
	{
	}

	EntityView* Entity::GetEntityView()
	{
		return m_pEntityScene->m_Registry.GetEntityView(m_EntityID);
	}

	void Entity::Clear()
	{
		m_pEntityScene->m_Registry.Clear(m_EntityID);
	}

	bool Entity::IsValid()
	{
		if (!m_pEntityScene) return false;
		return m_pEntityScene->m_Registry.IsValid(m_EntityID);
	}

	//void Entity::ForEachComponent(std::function<void(Registry*, EntityID, EntityComponentData*)> func)
	//{
	//	m_pEntityScene->m_Registry.ForEachComponent(m_EntityID, func);
	//}

	void Entity::Destroy()
	{
		if (!m_pEntityScene->IsValid()) return;
		m_pEntityScene->m_Registry.DestroyEntity(m_EntityID);
	}

	EntityID Entity::GetEntityID()
	{
		return m_EntityID;
	}

	EntityScene* Entity::GetScene()
	{
		return m_pEntityScene;
	}

	bool Entity::IsActiveSelf() const
	{
		return m_pEntityScene->m_Registry.GetEntityView(m_EntityID)->Active();
	}

	bool Entity::IsActive() const
	{
		return m_pEntityScene->m_Registry.GetEntityView(m_EntityID)->IsActive();
	}

	void Entity::SetActive(bool active)
	{
		m_pEntityScene->m_Registry.GetEntityView(m_EntityID)->Active() = active;
	}

	void Entity::SetActiveHierarchy(bool active)
	{
		m_pEntityScene->m_Registry.GetEntityView(m_EntityID)->HierarchyActive() = active;
	}
}
