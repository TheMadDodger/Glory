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
		virtual void OnUpdate(Registry* pRegistry, EntityID entity, T& pComponent) = 0;
		virtual void OnDraw(Registry* pRegistry, EntityID entity, T& pComponent) = 0;

	private:
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
