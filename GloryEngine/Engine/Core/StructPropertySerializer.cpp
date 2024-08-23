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

	void StructPropertySerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		const TypeData* pStructTypeData = Reflect::GetTyeData(typeHash);

		if (!name.empty())
			node[name].Set(YAML::Node(YAML::NodeType::Map));
		else
			node.Set(YAML::Node(YAML::NodeType::Map));

		for (size_t i = 0; i < pStructTypeData->FieldCount(); ++i)
		{
			const FieldData* pSubFieldData = pStructTypeData->GetFieldData(i);
			size_t offset = pSubFieldData->Offset();
			void* pAddress = (void*)((char*)(data)+offset);
			if (!name.empty())
				m_pSerializers->SerializeProperty(pSubFieldData, pAddress, node[name]);
			else
				m_pSerializers->SerializeProperty(pSubFieldData, pAddress, node[pSubFieldData->Name()]);
		}
	}

	void StructPropertySerializer::Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		if (!node.Exists() || !node.IsMap()) return;
		const TypeData* pStructTypeData = Reflect::GetTyeData(typeHash);
		for (size_t i = 0; i < pStructTypeData->FieldCount(); ++i)
		{
			const FieldData* pSubFieldData = pStructTypeData->GetFieldData(i);
			size_t offset = pSubFieldData->Offset();
			void* pAddress = (void*)((char*)(data)+offset);

			Utils::NodeValueRef structFieldObject = node[pSubFieldData->Name()];
			m_pSerializers->DeserializeProperty(pSubFieldData, pAddress, structFieldObject);
		}
	}
}
