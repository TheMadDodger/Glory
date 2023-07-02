#include "FieldData.h"
#include "Reflection.h"
#include <string>

namespace GloryReflect
{
	FieldData::FieldData(FieldType type, const char* name, const char* typeName, size_t offset, size_t size) :
		m_Type(Reflect::GetCustomTypeHash(type)),
		m_ElementType(type),
		m_Name(name),
		m_TypeName(typeName),
		m_Offset(offset),
		m_Size(size)
	{
		std::string tempTypeName = typeName;
		size_t templateIndex = tempTypeName.find('<');
		size_t templateEndIndex = tempTypeName.find('>');
		if (templateIndex == std::string::npos) return;
		std::string templateTypeName = tempTypeName.substr(0, templateIndex);
		std::string templatedTypeName = tempTypeName.substr(templateIndex + 1, templateEndIndex - templateIndex - 1 );
		const TypeData* pTemplateType = Reflect::GetTyeData(templateTypeName);
		const TypeData* pTemplatedType = Reflect::GetTyeData(templatedTypeName);

		if (pTemplateType == nullptr)
			throw std::exception("FieldData construction failed! template class or template type is not registered!");

		m_Type = pTemplateType->TypeHash();
		m_ElementType = pTemplatedType ? pTemplatedType->TypeHash() : 0;
	}

	FieldData::FieldData(FieldType type, ElementType elementType, const char* name, const char* typeName, size_t offset, size_t size) :
		m_Type(type),
		m_ElementType(elementType),
		m_Name(name),
		m_TypeName(typeName),
		m_Offset(offset),
		m_Size(size)
	{
	}

	FieldData::~FieldData()
	{
	}

	const FieldType FieldData::Type() const
	{
		return m_Type;
	}

	const ElementType FieldData::ArrayElementType() const
	{
		return m_ElementType;
	}

	const char* FieldData::Name() const
	{
		return m_Name;
	}

	const char* FieldData::TypeName() const
	{
		return m_TypeName;
	}

	const size_t FieldData::Offset() const
	{
		return m_Offset;
	}

	const size_t FieldData::Size() const
	{
		return m_Size;
	}

	void FieldData::Get(void* pAddress, void* out) const
	{
		char* p = (char*)pAddress;
		memcpy(out, p + m_Offset, m_Size);
	}

	void FieldData::Set(void* pAddress, void* value) const
	{
		char* p = (char*)pAddress;
		memcpy(p + m_Offset, value, m_Size);
	}

	void* FieldData::GetAddress(void* pAddress) const
	{
		char* p = (char*)pAddress;
		return p + m_Offset;
	}

	const FieldData* FieldData::GetArrayElementFieldData(size_t index) const
	{
		return Reflect::GetArrayElementData(this, index);
	}
}
