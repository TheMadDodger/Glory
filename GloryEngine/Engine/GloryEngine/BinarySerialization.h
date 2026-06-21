#pragma once
namespace Glory::Utils
{
	class BinaryStream;
	namespace Reflect
	{
		struct FieldData;
		struct TypeData;
	}

	void SerializeData(BinaryStream& container, const Utils::Reflect::TypeData* pTypeData, void* data);
	void SerializeData(BinaryStream& container, const Utils::Reflect::FieldData* pFieldData, void* data);
	void DeserializeData(BinaryStream& container, const Utils::Reflect::TypeData* pTypeData, void* data);
	void DeserializeData(BinaryStream& container, const Utils::Reflect::FieldData* pFieldData, void* data);
}