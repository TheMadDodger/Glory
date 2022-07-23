#include "PropertySerializer.h"

namespace Glory
{
	PropertySerializer* PropertySerializer::GetSerializer(size_t typeHash)
	{
		for (size_t i = 0; i < SERIALIZERS.size(); i++)
		{
			PropertySerializer* pSerializer = SERIALIZERS[i];
			size_t hash = pSerializer->GetSerializedTypeHash();
			if (hash == typeHash) return pSerializer;
		}

		return nullptr;
	}

	PropertySerializer* PropertySerializer::GetSerializer(const SerializedProperty* serializedProperty)
	{
		return GetSerializer(serializedProperty->TypeHash());
	}

	size_t PropertySerializer::GetID(PropertySerializer* pSerializer)
	{
		for (size_t i = 0; i < SERIALIZERS.size(); i++)
		{
			if (SERIALIZERS[i] == pSerializer) return i;
		}
		return 0;
	}

	PropertySerializer::PropertySerializer(size_t typeHash) : m_TypeHash(typeHash)
	{
	}

	PropertySerializer::~PropertySerializer()
	{
	}

	void PropertySerializer::SerializeProperty(const SerializedProperty* serializedProperty, YAML::Emitter& out)
	{
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(serializedProperty);
		if (pSerializer == nullptr) return;
		pSerializer->Serialize(serializedProperty, out);
	}

	void PropertySerializer::SerializeProperty(const std::string& name, const std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, YAML::Emitter& out)
	{
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(typeHash);
		if (pSerializer == nullptr) return;
		pSerializer->Serialize(name, buffer, typeHash, offset, size, out);
	}

	void PropertySerializer::DeserializeProperty(const SerializedProperty* serializedProperty, YAML::Node& object)
	{
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(serializedProperty);
		if (pSerializer == nullptr) return;
		pSerializer->Deserialize(serializedProperty, object);
	}

	void PropertySerializer::DeserializeProperty(std::any& out, size_t typeHash, YAML::Node& object)
	{
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(typeHash);
		if (pSerializer == nullptr) return;
		pSerializer->Deserialize(out, object);
	}

	void PropertySerializer::DeserializeProperty(std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, YAML::Node& object)
	{
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(typeHash);
		if (pSerializer == nullptr) return;
		pSerializer->Deserialize(buffer, offset, size, object);
	}

	size_t PropertySerializer::GetSerializedTypeHash() const
	{
		return m_TypeHash;
	}

	void PropertySerializer::Serialize(const std::string&, const std::vector<char>&, size_t, size_t, size_t, YAML::Emitter&) {}

	void PropertySerializer::Deserialize(std::vector<char>&, size_t, size_t, YAML::Node&) {}

	void PropertySerializer::Cleanup()
	{
		std::for_each(SERIALIZERS.begin(), SERIALIZERS.end(), [](PropertySerializer* pSerializer) { delete pSerializer; });
		SERIALIZERS.clear();
	}
}
