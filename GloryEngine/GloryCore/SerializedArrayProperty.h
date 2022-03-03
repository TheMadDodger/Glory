#pragma once
#include "SerializedProperty.h"
#include "SerializedPropertyManager.h"
#include "Resource.h"

namespace Glory
{
	class BaseSerializedArrayProperty : public SerializedProperty
	{
	public:
		BaseSerializedArrayProperty(UUID objectUUID, const std::string& name, void* pMember, uint32_t flags)
			: SerializedProperty(objectUUID, name, ST_Array, pMember, flags) {}

		virtual size_t ArraySize() const = 0;
		virtual SerializedProperty* GetArrayElementAt(size_t index) const = 0;
	};

	template<typename T, typename BaseT, typename TPropElem, typename... Args>
	class SerializedArrayProperty : public BaseSerializedArrayProperty
	{
	public:
		SerializedArrayProperty(UUID objectUUID, const std::string& name, std::vector<T>* pMember, uint32_t flags, Args&&... args)
			: BaseSerializedArrayProperty(objectUUID, name, pMember, flags), m_pMember(pMember), m_ElementTypeHash(ResourceType::GetHash<BaseT>())
		{
			T* pSubMember = pMember->data();
			for (size_t i = 0; i < pMember->size(); i++)
			{
				std::string propName = name + "_Element" + std::to_string(i);
				m_pSerializedArray.push_back(SerializedPropertyManager::GetProperty<TPropElem, T>(objectUUID, propName, pSubMember, flags, std::forward<Args>(args)...));
				++pSubMember;
			}
		}

		virtual size_t ArraySize() const override
		{
			return m_pSerializedArray.size();
		}

		virtual SerializedProperty* GetArrayElementAt(size_t index) const override
		{
			return m_pSerializedArray[index];
		}

		void Update(std::vector<T>* pMember)
		{
			m_pMember = pMember;
		}

	private:
		size_t m_ElementTypeHash;
		std::vector<T>* m_pMember;
		std::vector<SerializedProperty*> m_pSerializedArray;
	};
}
