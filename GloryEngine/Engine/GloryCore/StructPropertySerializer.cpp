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

	void StructPropertySerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out)
	{
		const GloryReflect::TypeData* pStructTypeData = GloryReflect::Reflect::GetTyeData(typeHash);

		if (!name.empty())
		{
			out << YAML::Key << name;
			out << YAML::Value;
		}

		out << YAML::BeginMap;
		for (size_t i = 0; i < pStructTypeData->FieldCount(); i++)
		{
			const GloryReflect::FieldData* pSubFieldData = pStructTypeData->GetFieldData(i);
			size_t offset = pSubFieldData->Offset();
			void* pAddress = (void*)((char*)(data)+offset);
			PropertySerializer::SerializeProperty(pSubFieldData, pAddress, out);
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
