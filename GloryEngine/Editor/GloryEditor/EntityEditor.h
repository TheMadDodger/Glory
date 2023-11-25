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
}
}
