#include "ArrayPropertySerializer.h"
#include <Reflection.h>

namespace Glory
{
	ArrayPropertySerializer::ArrayPropertySerializer() : PropertySerializer(SerializedType::ST_Array)
	{
	}

	ArrayPropertySerializer::~ArrayPropertySerializer()
	{
	}

	void ArrayPropertySerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out)
	{
		size_t size = GloryReflect::Reflect::ArraySize(data, typeHash);
		const GloryReflect::TypeData* pElementTypeData = GloryReflect::Reflect::GetTyeData(typeHash);

		if (!name.empty())
		{
			out << YAML::Key << name;
			out << YAML::Value;
		}

		out << YAML::BeginSeq;
		for (size_t i = 0; i < size; i++)
		{
			void* pElementAddress = GloryReflect::Reflect::ElementAddress(data, typeHash, i);
			PropertySerializer::SerializeProperty("", pElementTypeData, pElementAddress, out);
		}
		out << YAML::EndSeq;
	}

	void ArrayPropertySerializer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object)
	{
		const GloryReflect::TypeData* pElementTypeData = GloryReflect::Reflect::GetTyeData(typeHash);

		YAML::Node arrayNode = object;

		size_t size = arrayNode.size();
		GloryReflect::Reflect::ResizeArray(data, typeHash, size);
		for (size_t i = 0; i < size; i++)
		{
			YAML::Node elementNode = arrayNode[i];
			void* pElementAddress = GloryReflect::Reflect::ElementAddress(data, typeHash, i);
			PropertySerializer::DeserializeProperty(pElementTypeData, pElementAddress, elementNode);
		}
	}
}
