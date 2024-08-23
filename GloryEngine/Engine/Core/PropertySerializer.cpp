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

	void PropertySerializer::Serialize(const std::vector<char>&, uint32_t, size_t, size_t, Utils::NodeValueRef) {}

	void PropertySerializer::Serialize(void*, uint32_t, Utils::NodeValueRef) {}

	void PropertySerializer::Deserialize(std::vector<char>&, size_t, size_t, Utils::NodeValueRef) {}

	void PropertySerializer::Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef) {}
}
