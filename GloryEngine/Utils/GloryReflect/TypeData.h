#pragma once
#include "FieldData.h"
#include <string>

namespace GloryReflect
{
	struct TypeData
	{
	public:
		TypeData(const char* typeName, const FieldData* pFields, uint32_t internalTypeHash, uint32_t typeHash, int numFields);
		TypeData(const char* typeName, uint32_t enumTypeHash);
		virtual ~TypeData();

		const char* TypeName() const;
		const uint32_t InternalTypeHash() const;
		const uint32_t TypeHash() const;
		const size_t FieldCount() const;
		const FieldData* GetFieldData(size_t index) const;
		const FieldData* GetFieldData(const std::string& name) const;

	private:
		const char* m_TypeName;
		const uint32_t m_InternalTypeHash;
		const uint32_t m_TypeHash;
		const size_t m_FieldCount;
		const FieldData* m_pFields;
	};
}