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
		AssetReferencePropertyTemplate(UUID objectUUID, const std::string& name, UUID* pMember, uint32_t flags)
			: SerializedProperty(objectUUID, name, ST_Asset, pMember, flags)
		{
			m_ElementTypeHash = ResourceType::GetHash<T>();
		}

		void Update(UUID* pMember)
		{
			m_pMember = pMember;
		}
	};
}