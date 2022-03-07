#include "SerializedPropertyManager.h"
#include <algorithm>

namespace Glory
{
	std::map<UUID, std::map<std::string, SerializedProperty*>> SerializedPropertyManager::m_ManagedProperties;
	std::vector<SerializedProperty*> SerializedPropertyManager::m_AllProperties;

	void SerializedPropertyManager::Clear()
	{
		m_ManagedProperties.clear();
		std::for_each(m_AllProperties.begin(), m_AllProperties.end(), [](SerializedProperty* pProperty)
			{
				delete pProperty;
			});
		m_AllProperties.clear();
	}

	SerializedPropertyManager::SerializedPropertyManager() {}

	SerializedPropertyManager::~SerializedPropertyManager() {}
}
