#include "SerializedPropertyManager.h"

namespace Glory
{
	std::map<UUID, std::map<std::string, SerializedProperty*>> SerializedPropertyManager::m_ManagedProperties;

	void SerializedPropertyManager::Clear()
	{
		for (auto it1 = m_ManagedProperties.begin(); it1 != m_ManagedProperties.end(); it1++)
		{
			for (auto it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
			{
				delete it2->second;
			}
		}
		m_ManagedProperties.clear();
	}

	SerializedPropertyManager::SerializedPropertyManager() {}

	SerializedPropertyManager::~SerializedPropertyManager() {}
}
