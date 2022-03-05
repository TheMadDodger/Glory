#include "PropertySerializer.h"

namespace Glory
{
	std::vector<PropertySerializer*> PropertySerializer::m_pRegisteredSerializers;

	PropertySerializer* PropertySerializer::GetSerializer(size_t typeHash)
	{
		for (size_t i = 0; i < m_pRegisteredSerializers.size(); i++)
		{
			PropertySerializer* pSerializer = m_pRegisteredSerializers[i];
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
		for (size_t i = 0; i < m_pRegisteredSerializers.size(); i++)
		{
			if (m_pRegisteredSerializers[i] == pSerializer) return i;
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

	void PropertySerializer::DeserializeProperty(const SerializedProperty* serializedProperty, YAML::Node& object)
	{
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(serializedProperty);
		if (pSerializer == nullptr) return;
		pSerializer->Deserialize(serializedProperty, object);
	}

	size_t PropertySerializer::GetSerializedTypeHash() const
	{
		return m_TypeHash;
	}

	void PropertySerializer::Cleanup()
	{
		std::for_each(m_pRegisteredSerializers.begin(), m_pRegisteredSerializers.end(), [](PropertySerializer* pSerializer) { delete pSerializer; });
		m_pRegisteredSerializers.clear();
	}
}
