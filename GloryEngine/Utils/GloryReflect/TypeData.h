#pragma once
#include "FieldData.h"
#include <string>

namespace Glory::Utils::Reflect
{
	struct TypeData
	{
	public:
		TypeData(const char* typeName, const FieldData* pFields, uint32_t internalTypeHash, uint32_t typeHash, int numFields, int bufferOffset=-1, size_t bufferSize=0);
		TypeData(const char* typeName, uint32_t enumTypeHash);
		virtual ~TypeData();

		const char* TypeName() const;
		const uint32_t InternalTypeHash() const;
		const uint32_t TypeHash() const;
		const size_t FieldCount() const;
		const FieldData* GetFieldData(size_t index) const;
		const FieldData* GetFieldData(const std::string& name) const;
		const int DataBufferOffset() const;
		const size_t DataBufferSize() const;

	private:
		const char* m_TypeName;
		const uint32_t m_InternalTypeHash;
		const uint32_t m_TypeHash;
		const size_t m_FieldCount;
		const FieldData* m_pFields;
		const int m_BufferOffset;
		const size_t m_BufferSize;
	};
}