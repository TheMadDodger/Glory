#pragma once
#include <Object.h>
#include "EntityComponentData.h"

namespace Glory
{
	class EntityComponentObject : public Object
	{
	public:
		EntityComponentObject();
		EntityComponentObject(EntityComponentData* pComponentData, Registry* pRegistry);
		virtual ~EntityComponentObject();

		template<typename T>
		T& GetData()
		{
			return m_pComponentData->GetData<T>();
		}

		EntityComponentData* GetComponentData() const;
		Registry* GetRegistry() const;

	private:
		EntityComponentData* m_pComponentData;
		Registry* m_pRegistry;
	};
}
