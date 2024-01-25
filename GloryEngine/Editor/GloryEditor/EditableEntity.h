#pragma once
#include <Object.h>
#include <EntityID.h>

namespace Glory::Editor
{
	class EditableEntity : public Object
	{
	public:
		/** @brief Constructor */
		EditableEntity();
		/** @overload */
		EditableEntity(Utils::ECS::EntityID entity, UUID entityID, UUID sceneID);

		/** Should never be used */
		EditableEntity(UUID uuid, const std::string& name);

		Utils::ECS::EntityID EntityID() const;
		UUID SceneID() const;

	private:
		const Utils::ECS::EntityID m_Entity;
		const UUID m_SceneID;
	};
}

