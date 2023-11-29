#include "EditableEntity.h"

namespace Glory::Editor
{
	EditableEntity::EditableEntity(): m_Entity(0), m_SceneID(0)
	{
	}

	EditableEntity::EditableEntity(Utils::ECS::EntityID entity, UUID entityID, UUID sceneID):
		Object(entityID), m_Entity(entity), m_SceneID(sceneID)
	{
		APPEND_TYPE(EditableEntity);
	}

	Utils::ECS::EntityID EditableEntity::EntityID() const
	{
		return m_Entity;
	}

	UUID EditableEntity::SceneID() const
	{
		return m_SceneID;
	}
}
