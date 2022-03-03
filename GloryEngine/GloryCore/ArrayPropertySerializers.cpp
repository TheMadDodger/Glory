#include "ArrayPropertySerializers.h"
#include "SerializedArrayProperty.h"

namespace Glory
{
	ArrayPropertySerializers::ArrayPropertySerializers() : PropertySerializer(SerializedType::ST_Array)
	{
	}

	ArrayPropertySerializers::~ArrayPropertySerializers()
	{
	}

	void ArrayPropertySerializers::Serialize(const SerializedProperty* serializedProperty, YAML::Emitter& out)
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

	void ArrayPropertySerializers::Deserialize(const SerializedProperty* serializedProperty, YAML::Node& object)
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
}
