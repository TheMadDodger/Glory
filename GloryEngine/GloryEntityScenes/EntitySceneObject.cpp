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
			transform.Parent = nullptr;
			m_pParent = nullptr;
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

		Transform& parentTransform = parentHandle.GetComponent<Transform>();
		transform.Parent = &parentTransform;
	}

	Entity EntitySceneObject::GetEntityHandle()
	{
		return m_Entity;
	}

	void EntitySceneObject::Initialize()
	{
	}
}