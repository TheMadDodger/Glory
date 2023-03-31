#include "StructPropertySerializer.h"
#include <Reflection.h>

namespace Glory
{
	StructPropertySerializer::StructPropertySerializer() : PropertySerializer(SerializedType::ST_Struct)
	{
	}

	StructPropertySerializer::~StructPropertySerializer()
	{
	}

	void StructPropertySerializer::Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out)
	{
		void* pStructAddress = pFieldData->GetAddress(data);

		uint32_t structTypeHash = pFieldData->ArrayElementType();
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(structTypeHash);

		const GloryReflect::TypeData* pStructTypeData = GloryReflect::Reflect::GetTyeData(structTypeHash);
		if (pSerializer)
		{
			const GloryReflect::FieldData* pFieldData = pStructTypeData->GetFieldData(0);
			const GloryReflect::FieldData fieldData(structTypeHash, "", pFieldData->TypeName(), 0, pFieldData->Size());
			pSerializer->Serialize(&fieldData, pStructAddress, out);
			return;
		}

		out << YAML::Key << pFieldData->Name();
		out << YAML::Value << YAML::BeginMap;
		for (size_t i = 0; i < pStructTypeData->FieldCount(); i++)
		{
			const GloryReflect::FieldData* pSubFieldData = pStructTypeData->GetFieldData(i);
			PropertySerializer::SerializeProperty(pSubFieldData, pStructAddress, out);
		}
		out << YAML::EndMap;
	}

	void StructPropertySerializer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object)
	{
		const GloryReflect::TypeData* pStructTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
		for (size_t i = 0; i < pStructTypeData->FieldCount(); i++)
		{
			const GloryReflect::FieldData* pSubFieldData = pStructTypeData->GetFieldData(i);
			size_t offset = pSubFieldData->Offset();
			void* pAddress = (void*)((char*)(data)+offset);

			YAML::Node structFieldObject = object[pSubFieldData->Name()];
			PropertySerializer::DeserializeProperty(pSubFieldData, pAddress, structFieldObject);
		}
	}
}
