#include "EnumPropertySerializer.h"
#include <Reflection.h>

namespace Glory
{
	EnumPropertySerializer::EnumPropertySerializer() : PropertySerializer(SerializedType::ST_Enum)
	{
	}

	EnumPropertySerializer::~EnumPropertySerializer()
	{
	}

	void EnumPropertySerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out)
	{
		const GloryReflect::TypeData* pEnumTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
		GloryReflect::EnumType* pEnumType = GloryReflect::Reflect::GetEnumType(typeHash);
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
		const GloryReflect::TypeData* pEnumTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
		GloryReflect::EnumType* pEnumType = GloryReflect::Reflect::GetEnumType(typeHash);
		std::string valueString = object.as<std::string>();
		pEnumType->FromString(valueString, data);
	}
}
