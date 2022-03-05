#pragma once
#include <typeindex>
#include <exception>
#include <any>
#include <UUID.h>
#include "EntityID.h"

namespace Glory
{
	class EntityComponentData
	{
	public:
		EntityComponentData(EntityID entity, UUID uuid) : m_Entity(entity), m_UUID(uuid) {}
		virtual ~EntityComponentData() {}

		template<typename T, typename... Args>
		static EntityComponentData Construct(EntityID entity, UUID uuid, Args&&... args)
		{
			T data = T(args...);
			EntityComponentData componentData = EntityComponentData(entity, uuid);
			componentData.m_Data = data;
			return componentData;
		}

		std::type_index GetType()
		{
			return m_Data.type();
		}

		template<typename T>
		T& GetData()
		{
			return std::any_cast<T&>(m_Data);
		}

		UUID GetComponentUUID() const
		{
			return m_UUID;
		}

	private:
		friend class Registry;
		std::any m_Data;
		EntityID m_Entity;
		UUID m_UUID;
	};
}