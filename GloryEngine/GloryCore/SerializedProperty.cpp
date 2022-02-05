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

	size_t SerializedProperty::ElementTypeHash() const
	{
		return m_ElementTypeHash;
	}

	uint32_t SerializedProperty::Flags() const
	{
		return m_Flags;
	}

	void* SerializedProperty::MemberPointer() const
	{
		return m_pMember;
	}

	size_t SerializedProperty::ArrayElementsCount()
	{
		return m_ArrayElementsCount;
	}

	SerializedProperty SerializedProperty::GetArrayElementAt(size_t index)
	{
		std::string name = "Element" + std::to_string(index);
		char* pStart = (char*)m_pMember;
		char* pNewMeber = pStart + (index * m_ElementByteSize);
		return SerializedProperty(name, pNewMeber, m_ElementTypeHash, m_Flags);
	}

	Object* SerializedProperty::ObjectReference() const
	{
		// POINTER BLACK MAGIC!!!!! Kappa
		Object** pObjectMember = (Object**)m_pMember;
		return *pObjectMember;
	}
}
