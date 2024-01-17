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

	void EnumPropertySerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out)
	{
		const TypeData* pEnumTypeData = Reflect::GetTyeData(typeHash);
		EnumType* pEnumType = Reflect::GetEnumType(typeHash);
		std::string valueString;
		if(!pEnumType->ToString(data, valueString)) valueString = "none";
		if (name.empty())
		{
			out << valueString;
			return;
		}

		out << YAML::Key << name;
		out << YAML::Value << valueString;
	}

	void EnumPropertySerializer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object)
	{
		const TypeData* pEnumTypeData = Reflect::GetTyeData(typeHash);
		EnumType* pEnumType = Reflect::GetEnumType(typeHash);
		std::string valueString = object.as<std::string>();
		pEnumType->FromString(valueString, data);
	}
}
