#include "StructPropertySerializer.h"

#include <Reflection.h>

namespace Glory
{
	StructPropertySerializer::StructPropertySerializer(Serializers* pSerializers):
		PropertySerializer(pSerializers, SerializedType::ST_Struct)
	{
	}

	StructPropertySerializer::~StructPropertySerializer()
	{
	}

	void StructPropertySerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out)
	{
		const TypeData* pStructTypeData = Reflect::GetTyeData(typeHash);

		if (!name.empty())
		{
			out << YAML::Key << name;
			out << YAML::Value;
		}

		out << YAML::BeginMap;
		for (size_t i = 0; i < pStructTypeData->FieldCount(); i++)
		{
			const FieldData* pSubFieldData = pStructTypeData->GetFieldData(i);
			size_t offset = pSubFieldData->Offset();
			void* pAddress = (void*)((char*)(data)+offset);
			m_pSerializers->SerializeProperty(pSubFieldData, pAddress, out);
		}
		out << YAML::EndMap;
	}

	void StructPropertySerializer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object)
	{
		const TypeData* pStructTypeData = Reflect::GetTyeData(typeHash);
		for (size_t i = 0; i < pStructTypeData->FieldCount(); i++)
		{
			const FieldData* pSubFieldData = pStructTypeData->GetFieldData(i);
			size_t offset = pSubFieldData->Offset();
			void* pAddress = (void*)((char*)(data)+offset);

			YAML::Node structFieldObject = object[pSubFieldData->Name()];
			m_pSerializers->DeserializeProperty(pSubFieldData, pAddress, structFieldObject);
		}
	}
}
