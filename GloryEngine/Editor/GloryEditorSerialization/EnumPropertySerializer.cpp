#include "EnumPropertySerializer.h"
#include <Reflection.h>

namespace Glory
{
	EnumPropertySerializer::EnumPropertySerializer(Serializers* pSerializers):
		PropertySerializer(pSerializers, SerializedType::ST_Enum)
	{
	}

	EnumPropertySerializer::~EnumPropertySerializer()
	{
	}

	void EnumPropertySerializer::Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		const TypeData* pEnumTypeData = Reflect::GetTyeData(typeHash);
		EnumType* pEnumType = Reflect::GetEnumType(typeHash);
		std::string valueString;
		if(!pEnumType->ToString(data, valueString)) valueString = "none";
		node.Set(valueString);
	}

	void EnumPropertySerializer::Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		const TypeData* pEnumTypeData = Reflect::GetTyeData(typeHash);
		EnumType* pEnumType = Reflect::GetEnumType(typeHash);
		std::string valueString = node.As<std::string>();
		pEnumType->FromString(valueString, data);
	}
}
