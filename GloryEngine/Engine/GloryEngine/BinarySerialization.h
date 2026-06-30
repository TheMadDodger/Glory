#pragma once
#include "engine_visibility.h"

namespace Glory::Utils
{
	class BinaryStream;
	namespace Reflect
	{
		struct FieldData;
		struct TypeData;
	}

	GLORY_ENGINE_API void SerializeData(BinaryStream& container, const Utils::Reflect::TypeData* pTypeData, void* data);
	GLORY_ENGINE_API void SerializeData(BinaryStream& container, const Utils::Reflect::FieldData* pFieldData, void* data);
	GLORY_ENGINE_API void DeserializeData(BinaryStream& container, const Utils::Reflect::TypeData* pTypeData, void* data);
	GLORY_ENGINE_API void DeserializeData(BinaryStream& container, const Utils::Reflect::FieldData* pFieldData, void* data);
}