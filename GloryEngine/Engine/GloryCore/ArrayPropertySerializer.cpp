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

	void ArrayPropertySerializer::Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out)
	{
		void* pArrayAddress = pFieldData->GetAddress(data);

		uint32_t elementTypeHash = pFieldData->ArrayElementType();
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(elementTypeHash);

		size_t size = GloryReflect::Reflect::ArraySize(pArrayAddress, elementTypeHash);

		const GloryReflect::TypeData* pElementTypeData = GloryReflect::Reflect::GetTyeData(elementTypeHash);

		out << YAML::Key << pFieldData->Name();
		out << YAML::Value << YAML::BeginSeq;
		for (size_t i = 0; i < size; i++)
		{
			void* pElementAddress = GloryReflect::Reflect::ElementAddress(pArrayAddress, elementTypeHash, i);

			if (pSerializer)
			{
				const GloryReflect::FieldData* pFieldData = pElementTypeData->GetFieldData(0);
				const GloryReflect::FieldData fieldData(elementTypeHash, "", pFieldData->TypeName(), 0, pFieldData->Size());
				pSerializer->Serialize(&fieldData, pElementAddress, out);
				continue;
			}

			out << YAML::BeginMap;
			PropertySerializer::SerializeProperty(pElementTypeData, pElementAddress, out);
			out << YAML::EndMap;
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
