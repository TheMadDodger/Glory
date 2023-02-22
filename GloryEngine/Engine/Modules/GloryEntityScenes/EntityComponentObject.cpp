#include "EntityComponentObject.h"

namespace Glory
{
	EntityComponentObject::EntityComponentObject() : m_EntityID(NULL), m_ComponentType(NULL), m_pRegistry(nullptr)
	{
		APPEND_TYPE(EntityComponentObject);
	}

	EntityComponentObject::EntityComponentObject(GloryECS::EntityID entityID, UUID componentID, uint32_t componentType, GloryECS::EntityRegistry* pRegistry)
		: m_EntityID(entityID), m_ComponentType(componentType), m_pRegistry(pRegistry), Object(componentID)
	{
		APPEND_TYPE(EntityComponentObject);
		GloryECS::BaseTypeView* pTypeView = m_pRegistry->GetTypeView(componentType);
		PushInheritence(pTypeView->ComponentType());
	}

	EntityComponentObject::~EntityComponentObject()
	{
		m_EntityID = NULL;
		m_ComponentType = NULL;
		m_pRegistry = nullptr;
	}

	GloryECS::EntityRegistry* EntityComponentObject::GetRegistry() const
	{
		return m_pRegistry;
	}

	const GloryECS::EntityID EntityComponentObject::EntityID() const
	{
		return m_EntityID;
	}

	const uint32_t EntityComponentObject::ComponentType() const
	{
		return m_ComponentType;
	}

	void* EntityComponentObject::GetRootDataAddress()
	{
		return m_pRegistry->GetComponentAddress(m_EntityID, GetUUID());
	}
}
