#pragma once
#include <cstddef>
#include <typeindex>
#include <string_view>
#include "MacroHelpers.h"
#include "TypeData.h"

using namespace Glory::Utils::Reflect;

#pragma region Struct/Class Type Macros

#define REFLECTABLE_FIELD(...)\
ARGPAIR(__VA_ARGS__);

#define REFLECT_FIELD_INFO(x)\
FieldData(Reflect::Hash(typeid(ARGNAME(x))), ARGNAME_AS_STRING(x), ARGTYPE_AS_STRING(x), offsetof(TypeName, ARGNAME(x)), sizeof(ARGTYPE(x))),

#define REFLECTABLE_TYPEDATA(typeName, bufferOffset, bufferSize, ...)\
FOR_EACH(REFLECTABLE_FIELD, __VA_ARGS__)\
typedef typeName TypeName;\
public:\
	static const TypeData* GetTypeData()\
	{\
		static const char* typeNameString = STRINGIZE(typeName);\
		static const uint32_t TYPE_HASH = Reflect::Hash<typeName>();\
		static const int NUM_ARGS = NARGS(__VA_ARGS__);\
		static const FieldData pFields[] = {\
			FOR_EACH(REFLECT_FIELD_INFO, __VA_ARGS__)\
		};\
		static const TypeData pTypeData = TypeData(typeNameString, pFields, uint32_t(CustomTypeHash::Struct), TYPE_HASH, NUM_ARGS, bufferOffset, bufferSize);\
		return &pTypeData;\
	}

#define REFLECTABLE(typeName, ...)\
REFLECTABLE_TYPEDATA(typeName, -1, 0, __VA_ARGS__)\
	static int DataBufferOffset()\
	{\
		return -1;\
	}\
	static int DataBufferSize()\
	{\
		return 0;\
	}

#define REFLECTABLE_WITH_BUFFER(typeName, bufferMember, bufferSize, ...)\
REFLECTABLE_TYPEDATA(typeName, offsetof(typeName, bufferMember), bufferSize, __VA_ARGS__);\
	static int DataBufferOffset()\
	{\
		return offsetof(typeName, bufferMember);\
	}\
	static int DataBufferSize()\
	{\
		return bufferSize;\
	}


#pragma endregion

#pragma region Enum Type Macros

#define REFLECT_ENUM_VALUE(value) value,
#define REFLECT_ENUM_STRING_VALUE(value) STRINGIZE(value),

#define REFLECTABLE_ENUM(enumName, ...) enum class enumName									\
{																							\
	FOR_EACH(REFLECT_ENUM_VALUE, __VA_ARGS__)												\
};																							\
																							\
const std::string Enum<enumName>::m_EnumStringValues[] = {					\
	FOR_EACH(REFLECT_ENUM_STRING_VALUE, __VA_ARGS__)										\
};																							\
const size_t Enum<enumName>::m_NumValues = NARGS(__VA_ARGS__);				\
bool Enum<enumName>::Valid() { return true; }									\

#define REFLECTABLE_ENUM_NS(nameSpace, enumName, ...) namespace nameSpace {					\
enum class enumName																			\
{																							\
	FOR_EACH(REFLECT_ENUM_VALUE, __VA_ARGS__)												\
};																							\
}																							\
																							\
const std::string Enum<nameSpace::enumName>::m_EnumStringValues[] = {			\
	FOR_EACH(REFLECT_ENUM_STRING_VALUE, __VA_ARGS__)										\
};																							\
const size_t Enum<nameSpace::enumName>::m_NumValues = NARGS(__VA_ARGS__);		\
bool Enum<nameSpace::enumName>::Valid() { return true; }						\

#pragma endregion