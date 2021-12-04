#pragma once
#include <typeindex>
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

	private:
		friend class EntitySystems;
		Registry* m_pRegistry;
		std::type_index m_ComponentType;
	};

	template<typename T>
	class EntitySystemTemplate : public EntitySystem
	{
	public:
		EntitySystemTemplate(Registry* pRegistry) : EntitySystem(pRegistry, typeid(T)) {}
		virtual ~EntitySystemTemplate() {}

	protected:
		virtual void OnComponentAdded(Registry* pRegistry, EntityID entity, T& pComponent) {}
		virtual void OnComponentRemoved(Registry* pRegistry, EntityID entity, T& pComponent) {}

		virtual void OnUpdate(Registry* pRegistry, EntityID entity, T& pComponent) {}
		virtual void OnDraw(Registry* pRegistry, EntityID entity, T& pComponent) {}

	private:
		virtual void ComponentAdded(Registry* pRegistry, EntityID entity, EntityComponentData* pComponentData)
		{
			OnComponentAdded(pRegistry, entity, pComponentData->GetData<T>());
		}

		virtual void ComponentRemoved(Registry* pRegistry, EntityID entity, EntityComponentData* pComponentData)
		{
			OnComponentRemoved(pRegistry, entity, pComponentData->GetData<T>());
		}

		virtual void Update(Registry* pRegistry, EntityID entity, EntityComponentData* pComponentData) override
		{
			OnUpdate(pRegistry, entity, pComponentData->GetData<T>());
		}

		virtual void Draw(Registry* pRegistry, EntityID entity, EntityComponentData* pComponentData) override
		{
			OnDraw(pRegistry, entity, pComponentData->GetData<T>());
		}
	};
}
