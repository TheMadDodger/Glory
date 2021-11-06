#include "EntitySceneObject.h"

namespace Glory
{
	EntitySceneObject::EntitySceneObject(Entity entity) : m_Entity(entity)
	{
	}

	EntitySceneObject::EntitySceneObject(Entity entity, const std::string& name) : SceneObject(name), m_Entity(entity)
	{
	}

	EntitySceneObject::EntitySceneObject(Entity entity, const std::string& name, UUID uuid) : SceneObject(name, uuid), m_Entity(entity)
	{
	}

	EntitySceneObject::~EntitySceneObject()
	{
		m_Entity.Destroy();
	}

	Entity EntitySceneObject::GetEntityHandle()
	{
		return m_Entity;
	}

	void EntitySceneObject::Initialize()
	{
	}
}