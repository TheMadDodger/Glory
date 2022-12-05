#include "SerializedProperty.h"

namespace Glory
{
	SerializedProperty::SerializedProperty()
		: m_PropertyName("invalid"), m_pMember(nullptr), m_TypeHash(0), m_Flags(0) {}

	SerializedProperty::SerializedProperty(UUID objectUUID, const std::string& name, size_t typeHash, void* pMember, uint32_t flags)
		: m_ObjectUUID(objectUUID), m_PropertyName(name), m_pMember(pMember), m_TypeHash(typeHash), m_Flags(flags) {}

	SerializedProperty::SerializedProperty(UUID objectUUID, const std::string& name, size_t typeHash, size_t elementTypeHash, void* pMember, uint32_t flags)
		: m_ObjectUUID(objectUUID), m_PropertyName(name), m_pMember(pMember), m_TypeHash(typeHash), m_ElementTypeHash(elementTypeHash), m_Flags(flags) {}

	SerializedProperty::~SerializedProperty()
	{
		m_pMember = nullptr;
	}

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
		return m_ElementTypeHash != 0 ? m_ElementTypeHash : m_TypeHash;
	}

	uint32_t SerializedProperty::Flags() const
	{
		return m_Flags;
	}

	void* SerializedProperty::MemberPointer() const
	{
		return m_pMember;
	}

	//SerializedProperty SerializedProperty::GetArrayElementAt(size_t index)
	//{
	//	std::string name = "Element" + std::to_string(index);
	//	char* pStart = (char*)m_pMember;
	//	char* pNewMeber = pStart + (index * m_ElementByteSize);
	//	return SerializedProperty(name, pNewMeber, m_ElementTypeHash, m_Flags);
	//}

	Object* SerializedProperty::ObjectReference() const
	{
		// POINTER BLACK MAGIC!!!!! Kappa
		Object** pObjectMember = (Object**)m_pMember;
		return *pObjectMember;
	}
}
