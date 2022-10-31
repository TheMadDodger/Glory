#pragma once
#include <typeindex>
#include <exception>
#include <any>
#include <UUID.h>
#include "EntityID.h"
#include <Glory.h>

namespace Glory
{
	class EntityComponentData
	{
	public:
		GLORY_API EntityComponentData(EntityID entity, UUID uuid) : m_Entity(entity), m_UUID(uuid) {}
		GLORY_API virtual ~EntityComponentData() {}

		template<typename T, typename... Args>
		GLORY_API static EntityComponentData Construct(EntityID entity, UUID uuid, Args&&... args)
		{
			T data = T(args...);
			EntityComponentData componentData = EntityComponentData(entity, uuid);
			componentData.m_Data = data;
			return componentData;
		}

		GLORY_API std::type_index GetType()
		{
			return m_Data.type();
		}

		template<typename T>
		GLORY_API T& GetData()
		{
			const std::type_info& type = typeid(T);
			return std::any_cast<T&>(m_Data);
		}

		GLORY_API UUID GetComponentUUID() const
		{
			return m_UUID;
		}

		GLORY_API EntityComponentData& operator=(const EntityComponentData& other)
		{
			return *this;
		}

	private:
		friend class Registry;
		std::any m_Data;
		EntityID m_Entity;
		UUID m_UUID;
	};
}