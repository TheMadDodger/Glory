#include "BinarySerialization.h"

#include <BinaryStream.h>
#include <Reflection.h>
#include <EntityID.h>
#include <EntityRegistry.h>
#include <BinaryBuffer.h>

namespace Glory::Utils
{
	void SerializeData(BinaryStream& container, const Utils::Reflect::TypeData* pTypeData, void* data)
	{
		for (size_t i = 0; i < pTypeData->FieldCount(); ++i)
		{
			const Utils::Reflect::FieldData* pField = pTypeData->GetFieldData(i);
			void* pAddress = pField->GetAddress(data);
			SerializeData(container, pField, pAddress);
		}
	}

	void SerializeData(BinaryStream& container, const Utils::Reflect::FieldData* pFieldData, void* data)
	{
		const uint32_t type = pFieldData->Type();
		const uint32_t elementType = pFieldData->ArrayElementType();
		const Utils::Reflect::TypeData* pElementTypeData = Utils::Reflect::Reflect::GetTyeData(elementType);
		static const uint32_t stringType = Hashing::Hash(typeid(std::string).name());

		switch (type)
		{
		case uint32_t(CustomTypeHash::Struct): {
			const int bufferOffset = pElementTypeData->DataBufferOffset();
			const size_t bufferSize = pElementTypeData->DataBufferSize();
			for (size_t i = 0; i < pElementTypeData->FieldCount(); ++i)
			{
				const Utils::Reflect::FieldData* pField = pElementTypeData->GetFieldData(i);
				void* pAddress = pField->GetAddress(data);
				SerializeData(container, pField, pAddress);
			}
			if (bufferOffset != -1)
			{
				const char* pBuffer = (const char*)data + bufferOffset;
				container.Write(pBuffer, bufferSize);
			}
			break;
		}

		case uint32_t(CustomTypeHash::Array): {
			const size_t arraySize = Utils::Reflect::Reflect::ArraySize(data, elementType);
			container.Write(arraySize);
			for (size_t i = 0; i < arraySize; ++i)
			{
				const Utils::Reflect::FieldData* pElementField = pFieldData->GetArrayElementFieldData(i);
				void* pAddress = pElementField->GetAddress(data);
				SerializeData(container, pElementField, pAddress);
			}
			break;
		}
		case uint32_t(CustomTypeHash::Buffer): {
			BinaryBuffer* buffer = (BinaryBuffer*)data;
			container.Write(buffer->m_Buffer);
			break;
		}

		case uint32_t(CustomTypeHash::Enum):
		case uint32_t(CustomTypeHash::Basic):
		default: {
			if (elementType == stringType)
			{
				const std::string* str = static_cast<std::string*>(data);
				container.Write(*str);
				break;
			}
			const size_t size = pFieldData->Size();
			container.Write(data, size);
			break;
		}
		}
	}

	void DeserializeData(BinaryStream& container, const Utils::Reflect::TypeData* pTypeData, void* data)
	{
		for (size_t i = 0; i < pTypeData->FieldCount(); ++i)
		{
			const Utils::Reflect::FieldData* pField = pTypeData->GetFieldData(i);
			void* pAddress = pField->GetAddress(data);
			DeserializeData(container, pField, pAddress);
		}
	}

	void DeserializeData(BinaryStream& container, const Utils::Reflect::FieldData* pFieldData, void* data)
	{
		const uint32_t type = pFieldData->Type();
		const uint32_t elementType = pFieldData->ArrayElementType();
		const Utils::Reflect::TypeData* pElementTypeData = Utils::Reflect::Reflect::GetTyeData(elementType);
		static const uint32_t stringType = Hashing::Hash(typeid(std::string).name());

		switch (type)
		{
		case uint32_t(CustomTypeHash::Struct): {
			const int bufferOffset = pElementTypeData->DataBufferOffset();
			const size_t bufferSize = pElementTypeData->DataBufferSize();
			for (size_t i = 0; i < pElementTypeData->FieldCount(); ++i)
			{
				const Utils::Reflect::FieldData* pField = pElementTypeData->GetFieldData(i);
				void* pAddress = pField->GetAddress(data);
				DeserializeData(container, pField, pAddress);
			}
			if (bufferOffset != -1)
			{
				char* pBuffer = (char*)data + bufferOffset;
				container.Read(pBuffer, bufferSize);
			}
			break;
		}

		case uint32_t(CustomTypeHash::Array): {
			size_t arraySize;
			container.Read(arraySize);
			Utils::Reflect::Reflect::ResizeArray(data, elementType, arraySize);
			for (size_t i = 0; i < arraySize; ++i)
			{
				const Utils::Reflect::FieldData* pElementField = pFieldData->GetArrayElementFieldData(i);
				void* pAddress = pElementField->GetAddress(data);
				DeserializeData(container, pElementField, pAddress);
			}
			break;
		}
		case uint32_t(CustomTypeHash::Buffer): {
			BinaryBuffer* buffer = (BinaryBuffer*)data;
			size_t size;
			container.Read(size);
			if (size) container.Read(buffer->m_Buffer, size);
			break;
		}

		case uint32_t(CustomTypeHash::Enum):
		case uint32_t(CustomTypeHash::Basic):
		default: {
			if (elementType == stringType)
			{
				std::string* str = static_cast<std::string*>(data);
				container.Read(*str);
				break;
			}
			const size_t size = pFieldData->Size();
			container.Read(data, size);
			break;
		}
		}
	}
}