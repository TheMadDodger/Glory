#include "EntitySceneObject.h"
#include "Components.h"
#include <Debug.h>

namespace Glory
{
	EntitySceneObject::EntitySceneObject(Entity entity) : m_Entity(entity), m_pParent(nullptr)
	{
		APPEND_TYPE(EntitySceneObject);
	}

	EntitySceneObject::EntitySceneObject(Entity entity, const std::string& name) : SceneObject(name), m_Entity(entity), m_pParent(nullptr)
	{
		APPEND_TYPE(EntitySceneObject);
	}

	EntitySceneObject::EntitySceneObject(Entity entity, const std::string& name, UUID uuid) : SceneObject(name, uuid), m_Entity(entity), m_pParent(nullptr)
	{
		APPEND_TYPE(EntitySceneObject);
	}

	EntitySceneObject::~EntitySceneObject()
	{
		m_Entity.Destroy();
	}

	SceneObject* EntitySceneObject::GetParent()
	{
		return m_pParent;
	}

	void EntitySceneObject::OnSetParent(SceneObject* pParent)
	{
		if (!m_Entity.HasComponent<Transform>())
		{
			Debug::LogError("Cant add parent to an entity with no Transform component!");
			return;
		}

		Transform& transform = m_Entity.GetComponent<Transform>();

		if (pParent == nullptr)
		{
			transform.Parent = Entity();
			m_pParent = nullptr;
			SetHierarchyActive();
			return;
		}

		EntitySceneObject* pEntityParent = (EntitySceneObject*)pParent;
		Entity parentHandle = pEntityParent->GetEntityHandle(); 
		if (!parentHandle.HasComponent<Transform>())
		{
			Debug::LogError("Cant parent entity to an entity with no Transform component!");
			return;
		}

		m_pParent = pEntityParent;
		transform.Parent = parentHandle;
		SetHierarchyActive();
	}

	Entity EntitySceneObject::GetEntityHandle()
	{
		return m_Entity;
	}

	bool EntitySceneObject::IsActiveSelf() const
	{
		return m_Entity.IsActiveSelf();
	}

	bool EntitySceneObject::IsActiveInHierarchy() const
	{
		return m_Entity.IsActive();
	}

	void EntitySceneObject::SetActive(bool active)
	{
		m_Entity.SetActive(active);
		SetHierarchyActive();
	}

	void EntitySceneObject::SetHierarchyActive()
	{
		SceneObject* pParent = GetParent();
		const bool active = (!pParent || pParent->IsActiveInHierarchy()) && IsActiveSelf();
		m_Entity.SetActiveHierarchy(active);

		for (size_t i = 0; i < ChildCount(); i++)
		{
			GetChild(i)->SetHierarchyActive();
		}
	}

	void EntitySceneObject::Initialize()
	{
	}
}