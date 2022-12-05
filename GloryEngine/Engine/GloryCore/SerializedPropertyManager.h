#pragma once
#include "SerializedProperty.h"
#include "Object.h"
#include "Debug.h"
#include "GloryContext.h"
#include <map>

#define PROPERTY_MANAGER GloryContext::GetContext()->GetSerializedPropertyManager()

namespace Glory
{
	class SerializedPropertyManager
	{
	public:
		template<typename T, typename TMem, typename... Args>
		static SerializedProperty* GetProperty(UUID uuid, const std::string& propertyName, TMem* pMember, uint32_t flags, Args&&... args)
		{
			if (PROPERTY_MANAGER->m_ManagedProperties.find(uuid) == PROPERTY_MANAGER->m_ManagedProperties.end()
				|| PROPERTY_MANAGER->m_ManagedProperties[uuid].find(propertyName) == PROPERTY_MANAGER->m_ManagedProperties[uuid].end())
			{
				T* pNewProperty = new T(uuid, propertyName, pMember, flags, args...);
				PROPERTY_MANAGER->m_ManagedProperties[uuid][propertyName] = pNewProperty;
				PROPERTY_MANAGER->m_AllProperties.push_back(pNewProperty);
				return (SerializedProperty*)pNewProperty;
			}

			SerializedProperty* pProperty = PROPERTY_MANAGER->m_ManagedProperties[uuid][propertyName];
			T* pCastedProperty = (T*)pProperty;
			if (!pCastedProperty)
			{
				// This cant happen or we hef problem
				Debug::LogError("Incorrect SerializedProperty cast!");
				return nullptr;
			}

			pCastedProperty->Update(pMember, args...);
			return pCastedProperty;
		}

		static SerializedProperty* FindProperty(UUID uuid, const std::string& propertyName);

		static void Clear();

	private:
		std::map<UUID, std::map<std::string, SerializedProperty*>> m_ManagedProperties;
		std::vector<SerializedProperty*> m_AllProperties;

	private:
		friend class GloryContext;
		SerializedPropertyManager();
		virtual ~SerializedPropertyManager();
	};
}
