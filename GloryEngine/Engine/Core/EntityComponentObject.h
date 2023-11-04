#pragma once
#include "Object.h"
#include "Glory.h"
#include <EntityID.h>
#include <EntityRegistry.h>

namespace Glory
{
	class EntityComponentObject : public Object
	{
	public:
		EntityComponentObject();
		EntityComponentObject(Utils::ECS::EntityID entityID, UUID componentID, uint32_t componentType, Utils::ECS::EntityRegistry* pRegistry);
		virtual ~EntityComponentObject();

		template<typename T>
		T& GetData()
		{
			Glory::Utils::ECS::TypeView<T>* pTypeView = m_pRegistry->GetTypeView<T>();
			return pTypeView->Get(m_EntityID);
		}

		Utils::ECS::EntityRegistry* GetRegistry() const;
		const Utils::ECS::EntityID EntityID() const;
		const uint32_t ComponentType() const;

		virtual void* GetRootDataAddress() override;

	private:
		Utils::ECS::EntityID m_EntityID;
		uint32_t m_ComponentType;
		Utils::ECS::EntityRegistry* m_pRegistry;
	};
}
