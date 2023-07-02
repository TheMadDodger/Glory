#include "TypeData.h"
#include "CustomTypeHash.h"

namespace GloryReflect
{
	TypeData::TypeData(const char* typeName, const FieldData* pFields, uint32_t internalTypeHash, uint32_t typeHash, int numFields) :
		m_TypeName(typeName),
		m_pFields(pFields),
		m_TypeHash(typeHash),
		m_FieldCount(numFields),
		m_InternalTypeHash(internalTypeHash)
	{
	}

	TypeData::TypeData(const char* typeName, uint32_t enumTypeHash) :
		m_TypeName(typeName),
		m_pFields(new FieldData((size_t)CustomTypeHash::Enum, enumTypeHash, "m_value", typeName, 0, sizeof(size_t))),
		m_TypeHash(enumTypeHash),
		m_FieldCount(1),
		m_InternalTypeHash(uint32_t(CustomTypeHash::Enum))
	{
	}

	TypeData::~TypeData()
	{
	}

	const char* TypeData::TypeName() const
	{
		return m_TypeName;
	}

	const uint32_t TypeData::InternalTypeHash() const
	{
		return m_InternalTypeHash;
	}

	const uint32_t TypeData::TypeHash() const
	{
		return m_TypeHash;
	}

	const size_t TypeData::FieldCount() const
	{
		return m_FieldCount;
	}

	const FieldData* TypeData::GetFieldData(size_t index) const
	{
		return &m_pFields[index];
	}

	const FieldData* TypeData::GetFieldData(const std::string& name) const
	{
		for (size_t i = 0; i < m_FieldCount; ++i)
		{
			if (m_pFields[i].Name() != name) continue;
			return &m_pFields[i];
		}

		return nullptr;
	}
}
