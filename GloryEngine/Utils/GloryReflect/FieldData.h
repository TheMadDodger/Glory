#pragma once
#include <stdint.h>
#include <string>
#include <string_view>

namespace Glory::Utils::Reflect
{
	using FieldType = uint32_t;
	using ElementType = uint32_t;

	struct FieldData
	{
	public:
		FieldData(FieldType type, const char* name, const char* typeName, size_t offset, size_t size,
			const char* displayName=nullptr, const char* description=nullptr);
		FieldData(FieldType type, ElementType elementType, const char* name, const char* typeName, size_t offset, size_t size,
			const char* displayName=nullptr, const char* description=nullptr);
		virtual ~FieldData();

		const FieldType Type() const;
		const ElementType ArrayElementType() const;
		const std::string& Name() const;
		const std::string& TypeName() const;
		const std::string& DisplayName() const;
		const std::string& Description() const;
		const size_t Offset() const;
		const size_t Size() const;

		void Get(void* pAddress, void* out) const;
		void Set(void* pAddress, void* value) const;

		void* GetAddress(void* pAddress) const;

		const FieldData* GetArrayElementFieldData(size_t index) const;

	private:
		FieldType m_Type;
		ElementType m_ElementType;
		std::string m_Name;
		std::string m_TypeName;
		std::string m_DisplayName;
		std::string m_Description;
		size_t m_Offset;
		size_t m_Size;
	};
}
