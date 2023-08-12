#pragma once
#include <Object.h>
#include <Glory.h>
#include <EntityID.h>
#include <EntityRegistry.h>

namespace Glory
{
	class EntityComponentObject : public Object
	{
	public:
		GLORY_API EntityComponentObject();
		GLORY_API EntityComponentObject(Glory::Utils::ECS::EntityID entityID, UUID componentID, uint32_t componentType, Glory::Utils::ECS::EntityRegistry* pRegistry);
		GLORY_API virtual ~EntityComponentObject();

		template<typename T>
		GLORY_API T& GetData()
		{
			Glory::Utils::ECS::TypeView<T>* pTypeView = m_pRegistry->GetTypeView<T>();
			return pTypeView->Get(m_EntityID);
		}

		GLORY_API Glory::Utils::ECS::EntityRegistry* GetRegistry() const;
		GLORY_API const Glory::Utils::ECS::EntityID EntityID() const;
		GLORY_API const uint32_t ComponentType() const;

		GLORY_API virtual void* GetRootDataAddress() override;

	private:
		Glory::Utils::ECS::EntityID m_EntityID;
		uint32_t m_ComponentType;
		Glory::Utils::ECS::EntityRegistry* m_pRegistry;
	};
}
