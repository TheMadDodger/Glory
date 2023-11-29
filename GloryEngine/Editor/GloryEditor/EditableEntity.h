#pragma once
#include <Object.h>
#include <EntityID.h>

namespace Glory::Editor
{
	class EditableEntity : public Object
	{
	public:
		EditableEntity();
		EditableEntity(Utils::ECS::EntityID entity, UUID entityID, UUID sceneID);

		Utils::ECS::EntityID EntityID() const;
		UUID SceneID() const;

	private:
		const Utils::ECS::EntityID m_Entity;
		const UUID m_SceneID;
	};
}

