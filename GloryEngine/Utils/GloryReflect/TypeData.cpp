#include "TypeData.h"
#include "CustomTypeHash.h"

namespace Glory::Utils::Reflect
{
	TypeData::TypeData(const char* typeName, const FieldData* pFields, uint32_t internalTypeHash, uint32_t typeHash, int numFields, int bufferOffset, size_t bufferSize):
		m_TypeName(typeName),
		m_pFields(pFields),
		m_TypeHash(typeHash),
		m_FieldCount(numFields),
		m_InternalTypeHash(internalTypeHash),
		m_BufferOffset(bufferOffset),
		m_BufferSize(bufferSize)
	{
	}

	TypeData::TypeData(const char* typeName, uint32_t enumTypeHash) :
		m_TypeName(typeName),
		m_pFields(new FieldData((size_t)CustomTypeHash::Enum, enumTypeHash, "m_value", typeName, 0, sizeof(size_t))),
		m_TypeHash(enumTypeHash),
		m_FieldCount(1),
		m_InternalTypeHash(uint32_t(CustomTypeHash::Enum)),
		m_BufferOffset(-1),
		m_BufferSize(0)
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

	const int TypeData::DataBufferOffset() const
	{
		return m_BufferOffset;
	}

	const size_t TypeData::DataBufferSize() const
	{
		return m_BufferSize;
	}
}
