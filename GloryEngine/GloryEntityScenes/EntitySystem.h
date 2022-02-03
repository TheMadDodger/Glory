#pragma once
#include <typeindex>
#include <vector>
#include <SerializedProperty.h>
#include "EntityComponentData.h"

namespace Glory
{
	class Registry;

	class EntitySystem
	{
	public:
		EntitySystem(Registry* pRegistry, const std::type_index& type);
		virtual ~EntitySystem();

		virtual void ComponentAdded(Registry* pRegistry, EntityID entity, EntityComponentData* pComponentData) = 0;
		virtual void ComponentRemoved(Registry* pRegistry, EntityID entity, EntityComponentData* pComponentData) = 0;

		virtual void Update(Registry* pRegistry, EntityID entity, EntityComponentData* pComponentData) = 0;
		virtual void Draw(Registry* pRegistry, EntityID entity, EntityComponentData* pComponentData) = 0;
		virtual void AcquireSerializedProperties(EntityComponentData* pComponentData, std::vector<SerializedProperty>& properties) = 0;
		virtual void CreateComponent(EntityID entity) = 0;

	protected:
		friend class EntitySystems;
		Registry* m_pRegistry;
		std::type_index m_ComponentType;
	};
}
