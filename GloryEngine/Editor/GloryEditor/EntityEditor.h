#pragma once
#include <Object.h>
#include <EntityID.h>
#include <UUID.h>

#include "GloryEditor.h"

namespace Glory
{
	class GScene;
namespace Editor
{
	class EditableEntity;

	/** @brief Create an editable wrapper around an entity */
	GLORY_EDITOR_API EditableEntity* GetEditableEntity(Utils::ECS::EntityID entity, UUID sceneID);

	/** @overload */
	GLORY_EDITOR_API EditableEntity* GetEditableEntity(Utils::ECS::EntityID entity, GScene* pScene);

	/** @brief Destroy all instances of editable entities */
	GLORY_EDITOR_API void DestroyAllEditableEntities();

	/** @brief Destroy an entity from its scene and make sure it is not selected */
	GLORY_EDITOR_API void DestroyEntity(Utils::ECS::EntityID entity, GScene* pScene);
}
}
