#include "EnumPropertySerializers.h"
#include <Reflection.h>

namespace Glory
{
	EnumPropertySerializers::EnumPropertySerializers() : PropertySerializer(SerializedType::ST_Enum)
	{
	}

	EnumPropertySerializers::~EnumPropertySerializers()
	{
	}

	void EnumPropertySerializers::Serialize(const SerializedProperty* serializedProperty, YAML::Emitter& out)
	{
	}

	void EnumPropertySerializers::Deserialize(const SerializedProperty* serializedProperty, YAML::Node& object)
	{
	}

	void EnumPropertySerializers::Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out)
	{
		void* pEnumAddress = pFieldData->GetAddress(data);

		size_t enumTypeHash = pFieldData->ArrayElementType();
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

	void EnumPropertySerializers::Deserialize(std::any& out, YAML::Node& object)
	{
		
	}

	void EnumPropertySerializers::Deserialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object)
	{
		void* pEnumAddress = pFieldData->GetAddress(data);

		size_t enumTypeHash = pFieldData->ArrayElementType();
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
