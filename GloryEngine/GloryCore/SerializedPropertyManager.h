#pragma once
#include "SerializedProperty.h"
#include "Object.h"
#include "Debug.h"
#include <map>

namespace Glory
{
	class SerializedPropertyManager
	{
	public:
		template<typename T, typename TMem, typename... Args>
		static SerializedProperty* GetProperty(UUID uuid, const std::string& propertyName, TMem* pMember, uint32_t flags, Args&&... args)
		{
			if (m_ManagedProperties.find(uuid) == m_ManagedProperties.end()
				|| m_ManagedProperties[uuid].find(propertyName) == m_ManagedProperties[uuid].end())
			{
				T* pNewProperty = new T(uuid, propertyName, pMember, flags, args...);
				m_ManagedProperties[uuid][propertyName] = pNewProperty;
				return (SerializedProperty*)pNewProperty;
			}

			SerializedProperty* pProperty = m_ManagedProperties[uuid][propertyName];
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

		static void Clear();

	private:
		static std::map<UUID, std::map<std::string, SerializedProperty*>> m_ManagedProperties;

	private:
		SerializedPropertyManager();
		virtual ~SerializedPropertyManager();
	};
}
