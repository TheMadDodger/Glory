#include "SerializedPropertyManager.h"
#include <algorithm>

namespace Glory
{
	SerializedProperty* SerializedPropertyManager::FindProperty(UUID uuid, const std::string& propertyName)
	{
		if (PROPERTY_MANAGER->m_ManagedProperties.find(uuid) == PROPERTY_MANAGER->m_ManagedProperties.end()
			|| PROPERTY_MANAGER->m_ManagedProperties[uuid].find(propertyName) == PROPERTY_MANAGER->m_ManagedProperties[uuid].end()) return nullptr;

		SerializedProperty* pProperty = PROPERTY_MANAGER->m_ManagedProperties[uuid][propertyName];
		return pProperty;
	}

	void SerializedPropertyManager::Clear()
	{
		PROPERTY_MANAGER->m_ManagedProperties.clear();
		std::for_each(PROPERTY_MANAGER->m_AllProperties.begin(), PROPERTY_MANAGER->m_AllProperties.end(), [](SerializedProperty* pProperty)
		{
			delete pProperty;
		});
		PROPERTY_MANAGER->m_AllProperties.clear();
	}

	SerializedPropertyManager::SerializedPropertyManager() {}

	SerializedPropertyManager::~SerializedPropertyManager() {}
}
