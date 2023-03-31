#include "PropertySerializer.h"

namespace Glory
{
	PropertySerializer* PropertySerializer::GetSerializer(uint32_t typeHash)
	{
		for (size_t i = 0; i < PROPERTY_SERIALIZERS.size(); i++)
		{
			PropertySerializer* pSerializer = PROPERTY_SERIALIZERS[i];
			uint32_t hash = pSerializer->GetSerializedTypeHash();
			if (hash == typeHash) return pSerializer;
		}

		return nullptr;
	}

	size_t PropertySerializer::GetID(PropertySerializer* pSerializer)
	{
		for (size_t i = 0; i < PROPERTY_SERIALIZERS.size(); i++)
		{
			if (PROPERTY_SERIALIZERS[i] == pSerializer) return i;
		}
		return 0;
	}

	PropertySerializer::PropertySerializer(uint32_t typeHash) : m_TypeHash(typeHash)
	{
	}

	PropertySerializer::~PropertySerializer()
	{
	}

	void PropertySerializer::SerializeProperty(const std::string& name, const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, YAML::Emitter& out)
	{
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(typeHash);
		if (pSerializer == nullptr) return;
		pSerializer->Serialize(name, buffer, typeHash, offset, size, out);
	}

	void PropertySerializer::DeserializeProperty(std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, YAML::Node& object)
	{
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(typeHash);
		if (pSerializer == nullptr) return;
		pSerializer->Deserialize(buffer, offset, size, object);
	}

	void PropertySerializer::SerializeProperty(const std::string& name, const GloryReflect::TypeData* pTypeData, void* data, YAML::Emitter& out)
	{
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(pTypeData->TypeHash());
		PropertySerializer* pInternalSerializer = PropertySerializer::GetSerializer(pTypeData->InternalTypeHash());
		if (pSerializer)
		{
			pSerializer->Serialize(name, data, pTypeData->TypeHash(), out);
			return;
		}
		if (pInternalSerializer)
		{
			pInternalSerializer->Serialize(name, data, pTypeData->TypeHash(), out);
			return;
		}

		throw new std::exception("Missing serializer!");
	}

	void PropertySerializer::SerializeProperty(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out)
	{
		if (pFieldData->Type() == ST_Array)
		{
			return GetSerializer(ST_Array)->Serialize(pFieldData->Name(), data, pFieldData->ArrayElementType(), out);
		}

		const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(pFieldData->ArrayElementType());
		if (pTypeData)
		{
			SerializeProperty(pFieldData->Name(), pTypeData, data, out);
			return;
		}

		PropertySerializer* pSerializer = GetSerializer(pFieldData->Type());
		if (pSerializer)
		{
			pSerializer->Serialize(pFieldData->Name(), data, pFieldData->Type(), out);
			return;
		}

		throw new std::exception("Missing serializer!");
	}

	void PropertySerializer::DeserializeProperty(const GloryReflect::TypeData* pTypeData, void* data, YAML::Node& object)
	{
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(pTypeData->TypeHash());
		PropertySerializer* pInternalSerializer = PropertySerializer::GetSerializer(pTypeData->InternalTypeHash());
		if (pSerializer)
		{
			pSerializer->Deserialize(data, pTypeData->TypeHash(), object);
			return;
		}
		if (pInternalSerializer)
		{
			pInternalSerializer->Deserialize(data, pTypeData->TypeHash(), object);
			return;
		}

		throw new std::exception("Missing serializer!");
	}

	void PropertySerializer::DeserializeProperty(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object)
	{
		if (pFieldData->Type() == ST_Array)
		{
			return GetSerializer(ST_Array)->Deserialize(data, pFieldData->ArrayElementType(), object);
		}

		const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(pFieldData->ArrayElementType());
		if (pTypeData)
		{
			DeserializeProperty(pTypeData, data, object);
			return;
		}

		PropertySerializer* pSerializer = GetSerializer(pFieldData->Type());
		if (pSerializer)
		{
			pSerializer->Deserialize(data, pFieldData->Type(), object);
			return;
		}

		throw new std::exception("Missing serializer!");
	}

	uint32_t PropertySerializer::GetSerializedTypeHash() const
	{
		return m_TypeHash;
	}

	void PropertySerializer::Serialize(const std::string&, const std::vector<char>&, uint32_t, size_t, size_t, YAML::Emitter&) {}

	void PropertySerializer::Serialize(const std::string&, void*, uint32_t, YAML::Emitter&) {}

	void PropertySerializer::Deserialize(std::vector<char>&, size_t, size_t, YAML::Node&) {}

	void PropertySerializer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object) {}

	void PropertySerializer::Cleanup()
	{
		std::for_each(PROPERTY_SERIALIZERS.begin(), PROPERTY_SERIALIZERS.end(), [](PropertySerializer* pSerializer) { delete pSerializer; });
		PROPERTY_SERIALIZERS.clear();
	}
}
