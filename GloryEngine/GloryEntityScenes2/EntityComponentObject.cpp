//#include "EntityComponentObject.h"
//
//namespace Glory
//{
//	EntityComponentObject::EntityComponentObject() : m_pComponentData(nullptr), m_pRegistry(nullptr)
//	{
//		APPEND_TYPE(EntityComponentObject);
//	}
//
//	EntityComponentObject::EntityComponentObject(EntityComponentData* pComponentData, Registry* pRegistry)
//		: m_pComponentData(pComponentData), m_pRegistry(pRegistry), Object(pComponentData->GetComponentUUID())
//	{
//		APPEND_TYPE(EntityComponentObject);
//		PushInheritence(pComponentData->GetType());
//	}
//
//	EntityComponentObject::~EntityComponentObject()
//	{
//		m_pComponentData = nullptr;
//	}
//
//	EntityComponentData* EntityComponentObject::GetComponentData() const
//	{
//		return m_pComponentData;
//	}
//
//	Registry* EntityComponentObject::GetRegistry() const
//	{
//		return m_pRegistry;
//	}
//	UUID EntityComponentObject::GetComponentUUID() const
//	{
//		return m_pComponentData->GetComponentUUID();
//	}
//}
