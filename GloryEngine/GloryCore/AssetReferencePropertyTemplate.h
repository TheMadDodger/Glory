#pragma once
#include "SerializedProperty.h"

namespace Glory
{
	template<class T>
	class AssetReferencePropertyTemplate : public SerializedProperty
	{
	public:
		AssetReferencePropertyTemplate()
			: SerializedProperty(UUID(), "invalid", ST_Asset, nullptr)
		{
			m_ElementTypeHash = ResourceType::GetHash<T>();
		}
		AssetReferencePropertyTemplate(UUID objectUUID, const std::string& name, T** pMember, uint32_t flags)
			: SerializedProperty(objectUUID, name, ST_Asset, pMember, flags)
		{
			m_ElementTypeHash = ResourceType::GetHash<T>();
		}

		void Update(T** pMember)
		{
			m_pMember = pMember;
		}
	};
}