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

	void EnumPropertySerializer::Serialize(const SerializedProperty* serializedProperty, YAML::Emitter& out)
	{
	}

	void EnumPropertySerializer::Deserialize(const SerializedProperty* serializedProperty, YAML::Node& object)
	{
	}

	void EnumPropertySerializer::Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out)
	{
		void* pEnumAddress = pFieldData->GetAddress(data);

		uint32_t enumTypeHash = pFieldData->ArrayElementType();
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(enumTypeHash);

		const GloryReflect::TypeData* pEnumTypeData = GloryReflect::Reflect::GetTyeData(enumTypeHash);
		if (pSerializer)
		{
			const GloryReflect::FieldData* pFieldData = pEnumTypeData->GetFieldData(0);
			const GloryReflect::FieldData fieldData(enumTypeHash, "", pFieldData->TypeName(), 0, pFieldData->Size());
			pSerializer->Serialize(&fieldData, pEnumAddress, out);
			return;
		}

		GloryReflect::EnumType* pEnumType = GloryReflect::Reflect::GetEnumType(enumTypeHash);
		std::string valueString;
		if(!pEnumType->ToString(pEnumAddress, valueString)) valueString = "none";
		out << YAML::Key << pFieldData->Name();
		out << YAML::Value << valueString;
	}

	void EnumPropertySerializer::Deserialize(std::any& out, YAML::Node& object)
	{
		
	}

	void EnumPropertySerializer::Deserialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object)
	{
		void* pEnumAddress = pFieldData->GetAddress(data);

		uint32_t enumTypeHash = pFieldData->ArrayElementType();
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(enumTypeHash);

		const GloryReflect::TypeData* pEnumTypeData = GloryReflect::Reflect::GetTyeData(enumTypeHash);
		if (pSerializer)
		{
			const GloryReflect::FieldData* pFieldData = pEnumTypeData->GetFieldData(0);
			const GloryReflect::FieldData fieldData(enumTypeHash, "", pFieldData->TypeName(), 0, pFieldData->Size());
			pSerializer->Deserialize(&fieldData, pEnumAddress, object);
			return;
		}

		GloryReflect::EnumType* pEnumType = GloryReflect::Reflect::GetEnumType(enumTypeHash);
		std::string valueString = object.as<std::string>();
		pEnumType->FromString(valueString, pEnumAddress);
	}
}
