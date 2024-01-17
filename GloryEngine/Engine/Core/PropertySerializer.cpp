#include "PropertySerializer.h"

namespace Glory
{
	PropertySerializer::PropertySerializer(Serializers* pSerializers, uint32_t typeHash):
		m_pSerializers(pSerializers), m_TypeHash(typeHash)
	{
	}

	PropertySerializer::~PropertySerializer()
	{
	}

	uint32_t PropertySerializer::GetSerializedTypeHash() const
	{
		return m_TypeHash;
	}

	void PropertySerializer::Serialize(const std::string&, const std::vector<char>&, uint32_t, size_t, size_t, YAML::Emitter&) {}

	void PropertySerializer::Serialize(const std::string&, void*, uint32_t, YAML::Emitter&) {}

	void PropertySerializer::Deserialize(std::vector<char>&, size_t, size_t, YAML::Node&) {}

	void PropertySerializer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object) {}
}
