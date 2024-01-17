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

	void ArrayPropertySerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out)
	{
		size_t size = Reflect::ArraySize(data, typeHash);
		const TypeData* pElementTypeData = Reflect::GetTyeData(typeHash);

		if (!name.empty())
		{
			out << YAML::Key << name;
			out << YAML::Value;
		}

		out << YAML::BeginSeq;
		for (size_t i = 0; i < size; i++)
		{
			void* pElementAddress = Reflect::ElementAddress(data, typeHash, i);
			m_pSerializers->SerializeProperty("", pElementTypeData, pElementAddress, out);
		}
		out << YAML::EndSeq;
	}

	void ArrayPropertySerializer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object)
	{
		const TypeData* pElementTypeData = Reflect::GetTyeData(typeHash);

		YAML::Node arrayNode = object;

		size_t size = arrayNode.size();
		Reflect::ResizeArray(data, typeHash, size);
		for (size_t i = 0; i < size; i++)
		{
			YAML::Node elementNode = arrayNode[i];
			void* pElementAddress = Reflect::ElementAddress(data, typeHash, i);
			m_pSerializers->DeserializeProperty(pElementTypeData, pElementAddress, elementNode);
		}
	}
}
