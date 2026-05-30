#pragma once
#include "GloryEditor.h"

#include <Object.h>
#include <EntityID.h>
#include <EntityRegistry.h>

namespace Glory
{
	class EntityComponentObject : public Object
	{
	public:
		GLORY_EDITOR_API EntityComponentObject();
		GLORY_EDITOR_API EntityComponentObject(Utils::ECS::EntityID entityID, UUID componentID, uint32_t componentType, Utils::ECS::EntityRegistry* pRegistry);
		GLORY_EDITOR_API virtual ~EntityComponentObject();

		template<typename T>
		inline T& GetData()
		{
			return m_pRegistry->GetComponent<T>(m_EntityID);
		}

		GLORY_EDITOR_API Utils::ECS::EntityRegistry* GetRegistry() const;
		GLORY_EDITOR_API const Utils::ECS::EntityID EntityID() const;
		GLORY_EDITOR_API const uint32_t ComponentType() const;

		GLORY_EDITOR_API virtual void* GetRootDataAddress() override;

	private:
		Utils::ECS::EntityID m_EntityID;
		uint32_t m_ComponentType;
		Utils::ECS::EntityRegistry* m_pRegistry;
	};
}
