#pragma once
#include <Object.h>
#include "EntityComponentData.h"
#include <Glory.h>

namespace Glory
{
	class EntityComponentObject : public Object
	{
	public:
		GLORY_API EntityComponentObject();
		GLORY_API EntityComponentObject(EntityComponentData* pComponentData, Registry* pRegistry);
		GLORY_API virtual ~EntityComponentObject();

		template<typename T>
		GLORY_API T& GetData()
		{
			return m_pComponentData->GetData<T>();
		}

		GLORY_API EntityComponentData* GetComponentData() const;
		GLORY_API Registry* GetRegistry() const;

		GLORY_API UUID GetComponentUUID() const;

	private:
		EntityComponentData* m_pComponentData;
		Registry* m_pRegistry;
	};
}
