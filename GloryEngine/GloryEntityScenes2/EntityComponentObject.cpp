#include "EntityComponentObject.h"

namespace Glory
{
	EntityComponentObject::EntityComponentObject() : m_EntityID(NULL), m_ComponentType(NULL), m_pRegistry(nullptr)
	{
		APPEND_TYPE(EntityComponentObject);
	}

	EntityComponentObject::EntityComponentObject(GloryECS::EntityID entityID, UUID componentID, size_t componentType, GloryECS::EntityRegistry* pRegistry)
		: m_EntityID(entityID), m_pRegistry(pRegistry), Object(componentID)
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
}
