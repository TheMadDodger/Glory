#include "ArrayPropertySerializer.h"

#include <Reflection.h>

namespace Glory
{
	ArrayPropertySerializer::ArrayPropertySerializer(Serializers* pSerializers):
		PropertySerializer(pSerializers, SerializedType::ST_Array)
	{
	}

	ArrayPropertySerializer::~ArrayPropertySerializer()
	{
	}

	void ArrayPropertySerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		size_t size = Reflect::ArraySize(data, typeHash);
		const TypeData* pElementTypeData = Reflect::GetTyeData(typeHash);

		if (!name.empty())
			node[name].Set(YAML::Node(YAML::NodeType::Sequence));
		else
			node.Set(YAML::Node(YAML::NodeType::Sequence));

		for (size_t i = 0; i < size; ++i)
		{
			void* pElementAddress = Reflect::ElementAddress(data, typeHash, i);
			if (!name.empty())
				m_pSerializers->SerializeProperty("", pElementTypeData, pElementAddress, node[name][i]);
			else
				m_pSerializers->SerializeProperty("", pElementTypeData, pElementAddress, node[i]);
		}
	}

	void ArrayPropertySerializer::Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		if (!node.Exists() || !node.IsSequence()) return;

		const TypeData* pElementTypeData = Reflect::GetTyeData(typeHash);

		size_t size = node.Size();
		Reflect::ResizeArray(data, typeHash, size);
		for (size_t i = 0; i < size; ++i)
		{
			Utils::NodeValueRef elementNode = node[i];
			void* pElementAddress = Reflect::ElementAddress(data, typeHash, i);
			m_pSerializers->DeserializeProperty(pElementTypeData, pElementAddress, elementNode);
		}
	}
}
