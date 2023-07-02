#pragma once
#include <stdint.h>

namespace GloryReflect
{
	using FieldType = uint32_t;
	using ElementType = uint32_t;

	struct FieldData
	{
	public:
		FieldData(FieldType type, const char* name, const char* typeName, size_t offset, size_t size);
		FieldData(FieldType type, ElementType elementType, const char* name, const char* typeName, size_t offset, size_t size);
		virtual ~FieldData();

		const FieldType Type() const;
		const ElementType ArrayElementType() const;
		const char* Name() const;
		const char* TypeName() const;
		const size_t Offset() const;
		const size_t Size() const;

		void Get(void* pAddress, void* out) const;
		void Set(void* pAddress, void* value) const;

		void* GetAddress(void* pAddress) const;

		const FieldData* GetArrayElementFieldData(size_t index) const;

	private:
		FieldType m_Type;
		ElementType m_ElementType;
		const char* m_Name;
		const char* m_TypeName;
		size_t m_Offset;
		size_t m_Size;
	};
}
