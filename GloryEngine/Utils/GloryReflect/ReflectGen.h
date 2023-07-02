#pragma once
#include <cstddef>
#include <typeindex>
#include <string_view>
#include "MacroHelpers.h"
#include "TypeData.h"

#pragma region Struct/Class Type Macros

#define REFLECTABLE_FIELD(...)\
ARGPAIR(__VA_ARGS__);

#define REFLECT_FIELD_INFO(x)\
GloryReflect::FieldData(GloryReflect::Reflect::Hash(typeid(ARGNAME(x))), ARGNAME_AS_STRING(x), ARGTYPE_AS_STRING(x), offsetof(TypeName, ARGNAME(x)), sizeof(ARGTYPE(x))),

#define REFLECTABLE(typeName, ...)\
FOR_EACH(REFLECTABLE_FIELD, __VA_ARGS__)\
typedef typeName TypeName;\
public:\
	static const GloryReflect::TypeData* GetTypeData()\
	{\
		static const char* typeNameString = STRINGIZE(typeName);\
		static const uint32_t TYPE_HASH = GloryReflect::Reflect::Hash<typeName>();\
		static const int NUM_ARGS = NARGS(__VA_ARGS__);\
		static const GloryReflect::FieldData pFields[] = {\
			FOR_EACH(REFLECT_FIELD_INFO, __VA_ARGS__)\
		};\
		static const GloryReflect::TypeData pTypeData = GloryReflect::TypeData(typeNameString, pFields, uint32_t(GloryReflect::CustomTypeHash::Struct), TYPE_HASH, NUM_ARGS);\
		return &pTypeData;\
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
const std::string GloryReflect::Enum<enumName>::m_EnumStringValues[] = {					\
	FOR_EACH(REFLECT_ENUM_STRING_VALUE, __VA_ARGS__)										\
};																							\
const size_t GloryReflect::Enum<enumName>::m_NumValues = NARGS(__VA_ARGS__);				\
bool GloryReflect::Enum<enumName>::Valid() { return true; }									\

#define REFLECTABLE_ENUM_NS(nameSpace, enumName, ...) namespace nameSpace {					\
enum class enumName																			\
{																							\
	FOR_EACH(REFLECT_ENUM_VALUE, __VA_ARGS__)												\
};																							\
}																							\
																							\
const std::string GloryReflect::Enum<nameSpace::enumName>::m_EnumStringValues[] = {			\
	FOR_EACH(REFLECT_ENUM_STRING_VALUE, __VA_ARGS__)										\
};																							\
const size_t GloryReflect::Enum<nameSpace::enumName>::m_NumValues = NARGS(__VA_ARGS__);		\
bool GloryReflect::Enum<nameSpace::enumName>::Valid() { return true; }						\

#pragma endregion