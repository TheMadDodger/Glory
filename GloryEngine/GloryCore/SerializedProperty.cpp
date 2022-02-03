#include "SerializedProperty.h"

namespace Glory
{
	SerializedProperty::SerializedProperty()
		: m_PropertyName("invalid"), m_pMember(nullptr), m_TypeHash(0), m_Flags(0) {}

	SerializedProperty::SerializedProperty(const std::string& name, void* pMember, size_t typeHash, uint32_t flags)
		: m_PropertyName(name), m_pMember(pMember), m_TypeHash(typeHash), m_Flags(flags) {}

	const std::string& SerializedProperty::Name() const
	{
		return m_PropertyName;
	}

	size_t SerializedProperty::TypeHash() const
	{
		return m_TypeHash;
	}

	uint32_t SerializedProperty::Flags() const
	{
		return m_Flags;
	}

	void* SerializedProperty::MemberPointer() const
	{
		return m_pMember;
	}
}
