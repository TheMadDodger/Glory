#include "ArrayPropertySerializer.h"
#include "SerializedArrayProperty.h"
#include <Reflection.h>

namespace Glory
{
	ArrayPropertySerializer::ArrayPropertySerializer() : PropertySerializer(SerializedType::ST_Array)
	{
	}

	ArrayPropertySerializer::~ArrayPropertySerializer()
	{
	}

	void ArrayPropertySerializer::Serialize(const SerializedProperty* serializedProperty, YAML::Emitter& out)
	{
		BaseSerializedArrayProperty* serializedArrayProperty = (BaseSerializedArrayProperty*)serializedProperty;

		out << YAML::Key << serializedProperty->Name();
		out << YAML::Value << YAML::BeginMap;
		for (size_t i = 0; i < serializedArrayProperty->ArraySize(); i++)
		{
			const SerializedProperty* pChildProperty = serializedArrayProperty->GetArrayElementAt(i);
			PropertySerializer::SerializeProperty(pChildProperty, out);
		}
		out << YAML::EndMap;
	}

	void ArrayPropertySerializer::Deserialize(const SerializedProperty* serializedProperty, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		if (!object.IsMap()) return;

		BaseSerializedArrayProperty* serializedArrayProperty = (BaseSerializedArrayProperty*)serializedProperty;
		size_t size = object.size();
		serializedArrayProperty->Resize(size);

		for (size_t i = 0; i < serializedArrayProperty->ArraySize(); i++)
		{
			const SerializedProperty* pChildProperty = serializedArrayProperty->GetArrayElementAt(i);
			std::string name = pChildProperty->Name();
			YAML::Node childNode = object[name];
			if (!childNode.IsDefined()) continue;
			PropertySerializer::DeserializeProperty(pChildProperty, childNode);
		}
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

	void ArrayPropertySerializer::Deserialize(std::any& out, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		if (!object.IsMap()) return;


	}

	void ArrayPropertySerializer::Deserialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object)
	{
		void* pArrayAddress = pFieldData->GetAddress(data);

		uint32_t elementTypeHash = pFieldData->ArrayElementType();
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(elementTypeHash);

		const GloryReflect::TypeData* pElementTypeData = GloryReflect::Reflect::GetTyeData(elementTypeHash);

		YAML::Node arrayNode = object;

		size_t size = arrayNode.size();
		GloryReflect::Reflect::ResizeArray(pArrayAddress, elementTypeHash, size);
		for (size_t i = 0; i < size; i++)
		{
			YAML::Node elementNode = arrayNode[i];

			void* pElementAddress = GloryReflect::Reflect::ElementAddress(pArrayAddress, elementTypeHash, i);

			if (pSerializer)
			{
				const GloryReflect::FieldData* pFieldData = pElementTypeData->GetFieldData(0);
				const GloryReflect::FieldData fieldData(elementTypeHash, "", pFieldData->TypeName(), 0, pFieldData->Size());
				pSerializer->Deserialize(&fieldData, pElementAddress, elementNode);
				continue;
			}

			PropertySerializer::DeserializeProperty(pElementTypeData, pElementAddress, elementNode);
		}
	}
}
