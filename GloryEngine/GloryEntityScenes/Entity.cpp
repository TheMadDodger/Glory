#include "Entity.h"

namespace Glory
{
	Entity::Entity() : m_EntityID(NULL), m_pEntityScene(nullptr)
	{
	}

	Entity::Entity(EntityID entityHandle, EntityScene* pScene) : m_EntityID(entityHandle), m_pEntityScene(pScene)
	{
	}

	void Entity::Clear()
	{
		m_pEntityScene->m_Registry.Clear(m_EntityID);
	}

	bool Entity::IsValid()
	{
		return m_pEntityScene->m_Registry.IsValid(m_EntityID);
	}

	void Entity::ForEachComponent(std::function<void(Registry*, EntityID, EntityComponentData*)> func)
	{
		m_pEntityScene->m_Registry.ForEachComponent(m_EntityID, func);
	}
}
