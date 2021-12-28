#include "EntityComponentObject.h"

namespace Glory
{
	EntityComponentObject::EntityComponentObject() : m_pComponentData(nullptr)
	{
		APPEND_TYPE(EntityComponentObject);
	}

	EntityComponentObject::EntityComponentObject(EntityComponentData* pComponentData) : m_pComponentData(pComponentData)
	{
		APPEND_TYPE(EntityComponentObject);
		PushInheritence(pComponentData->GetType());
	}

	EntityComponentObject::~EntityComponentObject()
	{
		m_pComponentData = nullptr;
	}
}
