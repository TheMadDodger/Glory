#pragma once
#include <typeindex>
#include <exception>
#include "EntityID.h"
#include <any>

namespace Glory
{
	class EntityComponentData
	{
	public:
		EntityComponentData(EntityID entity) : m_Entity(entity) {}

		template<typename T, typename... Args>
		static EntityComponentData Construct(EntityID entity, Args&&... args)
		{
			T data = T(args...);
			EntityComponentData componentData = EntityComponentData(entity);
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

	private:
		friend class Registry;
		std::any m_Data;
		EntityID m_Entity;
	};
}