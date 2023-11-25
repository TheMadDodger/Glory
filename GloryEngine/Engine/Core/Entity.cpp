#include "Entity.h"
#include "GScene.h"

namespace Glory
{
	Entity::Entity() : m_EntityID(NULL), m_pGScene(nullptr), m_pRegistry(nullptr)
	{
	}

	Entity::Entity(Utils::ECS::EntityID entityHandle, GScene* pScene) :
		m_EntityID(entityHandle), m_pGScene(pScene), m_pRegistry(&pScene->m_Registry)
	{
	}

	Entity Entity::ParentEntity() const
	{
		return m_pGScene->GetEntityByEntityID(Parent());
	}

	Utils::ECS::EntityID Entity::Parent() const
	{
		return m_pRegistry->GetParent(m_EntityID);
	}

	void Entity::SetParent(Utils::ECS::EntityID parent)
	{
		m_pRegistry->SetParent(m_EntityID, parent);
	}

	Utils::ECS::EntityView* Entity::GetEntityView() const
	{
		return m_pRegistry->GetEntityView(m_EntityID);
	}

	void Entity::Clear()
	{
		m_pRegistry->Clear(m_EntityID);
	}

	bool Entity::IsValid() const
	{
		if (!m_pGScene) return false;
		return m_pRegistry->IsValid(m_EntityID);
	}

	void Entity::Destroy()
	{
		if (!m_pGScene) return;
		m_pRegistry->DestroyEntity(m_EntityID);
	}

	Utils::ECS::EntityID Entity::GetEntityID() const
	{
		return m_EntityID;
	}

	UUID Entity::EntityUUID() const
	{
		return IsValid() ? m_pGScene->GetEntityUUID(m_EntityID) : UUID(0);
	}

	size_t Entity::ChildCount() const
	{
		return IsValid() ? m_pRegistry->ChildCount(m_EntityID) : 0;
	}

	Entity Entity::ChildEntity(size_t index) const
	{
		return m_pGScene->GetEntityByEntityID(Child(index));
	}

	Utils::ECS::EntityID Entity::Child(size_t index) const
	{
		return IsValid() ? m_pRegistry->Child(m_EntityID, index) : 0;
	}

	size_t Entity::SiblingIndex() const
	{
		return IsValid() ? m_pRegistry->SiblingIndex(m_EntityID) : 0;
	}

	void Entity::SetSiblingIndex(size_t index)
	{
		m_pRegistry->SetSiblingIndex(m_EntityID, index);
	}

	Utils::ECS::EntityRegistry* Entity::GetRegistry() const
	{
		return m_pRegistry;
	}

	GScene* Entity::GetScene() const
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
