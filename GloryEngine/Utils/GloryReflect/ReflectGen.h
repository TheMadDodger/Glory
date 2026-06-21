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
	inline static const TypeData* GetTypeData()\
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

#define GET_TYPE(type, ...) type
#define GET_NAME(type, name, ...) name
#define GET_DISPLAYNAME(type, name, displayname, ...) displayname
#define GET_DESCRIPTION(type, name, displayname, description) description

#define REFLECTABLE_FIELD_DESCRIPTIVE(x)\
GET_TYPE x GET_NAME x;

#define REFLECT_FIELD_INFO_DESCRIPTIVE(x)\
FieldData(Reflect::Hash(typeid(GET_TYPE x)), STRINGIZE(GET_NAME x), STRINGIZE(GET_TYPE x), offsetof(TypeName, GET_NAME x), sizeof(GET_TYPE x), GET_DISPLAYNAME x, GET_DESCRIPTION x),

#define REFLECTABLE_TYPEDATA_DESCRIPTIVE(typeName, bufferOffset, bufferSize, ...)\
FOR_EACH(REFLECTABLE_FIELD_DESCRIPTIVE, __VA_ARGS__)\
typedef typeName TypeName;\
public:\
	inline static const TypeData* GetTypeData()\
	{\
		static const char* typeNameString = STRINGIZE(typeName);\
		static const uint32_t TYPE_HASH = Reflect::Hash<typeName>();\
		static const int NUM_ARGS = NARGS(__VA_ARGS__);\
		static const FieldData pFields[] = {\
			FOR_EACH(REFLECT_FIELD_INFO_DESCRIPTIVE, __VA_ARGS__)\
		};\
		static const TypeData pTypeData = TypeData(typeNameString, pFields, uint32_t(CustomTypeHash::Struct), TYPE_HASH, NUM_ARGS, bufferOffset, bufferSize);\
		return &pTypeData;\
	}

#define REFLECTABLE(typeName, ...)\
REFLECTABLE_TYPEDATA(typeName, -1, 0, __VA_ARGS__)\
	inline static int DataBufferOffset()\
	{\
		return -1;\
	}\
	inline static int DataBufferSize()\
	{\
		return 0;\
	}

#define REFLECTABLE_DESCRIPTIVE(typeName, ...)\
REFLECTABLE_TYPEDATA_DESCRIPTIVE(typeName, -1, 0, __VA_ARGS__)\
	inline static int DataBufferOffset()\
	{\
		return -1;\
	}\
	inline static int DataBufferSize()\
	{\
		return 0;\
	}

#define REFLECTABLE_WITH_BUFFER(typeName, bufferMember, bufferSize, ...)\
REFLECTABLE_TYPEDATA(typeName, offsetof(typeName, bufferMember), bufferSize, __VA_ARGS__);\
	inline static int DataBufferOffset()\
	{\
		return offsetof(typeName, bufferMember);\
	}\
	inline static int DataBufferSize()\
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
template<>																					\
inline const std::string Enum<enumName>::m_EnumStringValues[] = {							\
	FOR_EACH(REFLECT_ENUM_STRING_VALUE, __VA_ARGS__)										\
};																							\
template<>																					\
inline const size_t Enum<enumName>::m_NumValues = NARGS(__VA_ARGS__);						\
template<>																					\
inline bool Enum<enumName>::Valid() { return true; }										\

#define REFLECTABLE_ENUM_NS(nameSpace, enumName, ...) namespace nameSpace {					\
enum class enumName																			\
{																							\
	FOR_EACH(REFLECT_ENUM_VALUE, __VA_ARGS__)												\
};																							\
}																							\
																							\
template<>																					\
inline const std::string Enum<nameSpace::enumName>::m_EnumStringValues[] = {				\
	FOR_EACH(REFLECT_ENUM_STRING_VALUE, __VA_ARGS__)										\
};																							\
template<>																					\
inline const size_t Enum<nameSpace::enumName>::m_NumValues = NARGS(__VA_ARGS__);			\
template<>																					\
inline bool Enum<nameSpace::enumName>::Valid() { return true; }								\


#define REFLECTABLE_ENUM_NS_SIZE(nameSpace, enumName, size, ...) namespace nameSpace {		\
enum class enumName	: size																	\
{																							\
	FOR_EACH(REFLECT_ENUM_VALUE, __VA_ARGS__)												\
};																							\
}																							\
																							\
template<>																					\
inline const std::string Enum<nameSpace::enumName>::m_EnumStringValues[] = {				\
	FOR_EACH(REFLECT_ENUM_STRING_VALUE, __VA_ARGS__)										\
};																							\
template<>																					\
inline const size_t Enum<nameSpace::enumName>::m_NumValues = NARGS(__VA_ARGS__);			\
template<>																					\
inline bool Enum<nameSpace::enumName>::Valid() { return true; }								\

#define PROP(type, name) (type, m_##name, STRINGIZE(name), "")
#define PROP_TOOLTIP(type, name, tooltip) (type, m_##name, STRINGIZE(name), tooltip)

#pragma endregion