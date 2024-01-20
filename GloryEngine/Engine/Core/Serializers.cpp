#include "Serializers.h"
#include "ResourceType.h"
#include "SerializedTypes.h"
#include "GScene.h"
#include "PropertySerializer.h"

namespace Glory
{
	/* This ensures legacy support by rerouting an old hash to a new one */
	const std::map<uint32_t, uint32_t> LegacyRehasher = {
		{ 3536807891, ResourceTypes::GetHash<GScene>() },
		{ 1002038608, SerializedType::ST_Object },
		{ 201832386, 1226719936 }
	};

	Serializers::Serializers()
	{
	}

	Serializers::~Serializers()
	{
		std::for_each(m_pRegisteredPropertySerializers.begin(), m_pRegisteredPropertySerializers.end(), [](PropertySerializer* pSerializer) { delete pSerializer; });
		m_pRegisteredPropertySerializers.clear();
	}

	PropertySerializer* Serializers::GetSerializer(uint32_t typeHash)
	{
		for (size_t i = 0; i < m_pRegisteredPropertySerializers.size(); i++)
		{
			PropertySerializer* pSerializer = m_pRegisteredPropertySerializers[i];
			uint32_t hash = pSerializer->GetSerializedTypeHash();
			if (hash == typeHash) return pSerializer;
		}

		return nullptr;
	}

	size_t Serializers::GetID(PropertySerializer* pSerializer)
	{
		for (size_t i = 0; i < m_pRegisteredPropertySerializers.size(); ++i)
		{
			if (m_pRegisteredPropertySerializers[i] == pSerializer) return i;
		}
		return 0;
	}

	void Serializers::SerializeProperty(const std::string& name, const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, YAML::Emitter& out)
	{
		PropertySerializer* pSerializer = GetSerializer(typeHash);
		if (pSerializer == nullptr) return;
		pSerializer->Serialize(name, buffer, typeHash, offset, size, out);
	}

	void Serializers::DeserializeProperty(std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, YAML::Node& object)
	{
		PropertySerializer* pSerializer = GetSerializer(typeHash);
		if (pSerializer == nullptr) return;
		pSerializer->Deserialize(buffer, offset, size, object);
	}

	void Serializers::SerializeProperty(const std::string& name, const Utils::Reflect::TypeData* pTypeData, void* data, YAML::Emitter& out)
	{
		PropertySerializer* pSerializer = GetSerializer(pTypeData->TypeHash());
		PropertySerializer* pInternalSerializer = GetSerializer(pTypeData->InternalTypeHash());
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

	void Serializers::SerializeProperty(const FieldData* pFieldData, void* data, YAML::Emitter& out)
	{
		if (pFieldData->Type() == ST_Array)
		{
			return GetSerializer(ST_Array)->Serialize(pFieldData->Name(), data, pFieldData->ArrayElementType(), out);
		}

		const Utils::Reflect::TypeData* pTypeData = Reflect::GetTyeData(pFieldData->ArrayElementType());
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

	void Serializers::DeserializeProperty(const Utils::Reflect::TypeData* pTypeData, void* data, YAML::Node& object)
	{
		PropertySerializer* pSerializer = GetSerializer(pTypeData->TypeHash());
		PropertySerializer* pInternalSerializer = GetSerializer(pTypeData->InternalTypeHash());
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

	void Serializers::DeserializeProperty(const FieldData* pFieldData, void* data, YAML::Node& object)
	{
		if (pFieldData->Type() == ST_Array)
		{
			return GetSerializer(ST_Array)->Deserialize(data, pFieldData->ArrayElementType(), object);
		}

		const Utils::Reflect::TypeData* pTypeData = Reflect::GetTyeData(pFieldData->ArrayElementType());
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
}
