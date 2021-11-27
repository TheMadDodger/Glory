#pragma once
#include <Object.h>
#include "EntityComponentData.h"

namespace Glory
{
	class EntityComponentObject : public Object
	{
	public:
		EntityComponentObject();
		EntityComponentObject(EntityComponentData* pComponentData);
		virtual ~EntityComponentObject();

		template<typename T>
		T& GetData()
		{
			return m_pComponentData->GetData<T>();
		}

	private:
		EntityComponentData* m_pComponentData;
	};
}
