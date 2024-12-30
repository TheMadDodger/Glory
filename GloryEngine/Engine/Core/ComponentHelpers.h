#pragma once
#include "Glory.h"
#include "Entity.h"

namespace Glory::Components
{
	/** @brief Activate an entity */
	GLORY_API inline void Activate(Entity entity);
	/** @brief Deactivate an entity */
	GLORY_API inline void Deactivate(Entity entity);
	/** @brief Activate a component */
	GLORY_API inline void Activate(Entity entity, Utils::ECS::BaseTypeView* pTypeView, size_t componentIndex);
	/** @brief Trigger OnEnable on all components of an entity that are already active */
	GLORY_API inline void CallOnEnable(Entity entity);
	/** @brief Trigger OnEnable if the component is already active */
	GLORY_API inline void CallOnEnable(Entity entity, Utils::ECS::BaseTypeView* pTypeView, size_t componentIndex);
	/** @brief Deactivate a component */
	GLORY_API inline void Deactivate(Entity entity, Utils::ECS::BaseTypeView* pTypeView, size_t componentIndex);
	/** @brief Trigger OnDisable on all components of an entity that are already inactive */
	GLORY_API inline void CallOnDisable(Entity entity);
	/** @brief Trigger OnDisable if the component is already inactive */
	GLORY_API inline void CallOnDisable(Entity entity, Utils::ECS::BaseTypeView* pTypeView, size_t componentIndex);
	/** @brief Destroy/remove a component */
	GLORY_API inline bool Destroy(Entity entity, Utils::ECS::BaseTypeView* pTypeView, size_t componentIndex);
}