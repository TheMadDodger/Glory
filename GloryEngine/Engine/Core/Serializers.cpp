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

	Serializers::Serializers(Engine* pEngine): m_pEngine(pEngine)
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

	void Serializers::SerializeProperty(const std::string& name, const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, Utils::NodeValueRef node)
	{
		PropertySerializer* pSerializer = GetSerializer(typeHash);
		if (pSerializer == nullptr) return;
		pSerializer->Serialize(name, buffer, typeHash, offset, size, node);
	}

	void Serializers::DeserializeProperty(std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, Utils::NodeValueRef node)
	{
		PropertySerializer* pSerializer = GetSerializer(typeHash);
		if (pSerializer == nullptr) return;
		pSerializer->Deserialize(buffer, offset, size, node);
	}

	void Serializers::SerializeProperty(const std::string& name, const Utils::Reflect::TypeData* pTypeData, void* data, Utils::NodeValueRef node)
	{
		PropertySerializer* pSerializer = GetSerializer(pTypeData->TypeHash());
		PropertySerializer* pInternalSerializer = GetSerializer(pTypeData->InternalTypeHash());
		if (pSerializer)
		{
			pSerializer->Serialize(name, data, pTypeData->TypeHash(), node);
			return;
		}
		if (pInternalSerializer)
		{
			pInternalSerializer->Serialize(name, data, pTypeData->TypeHash(), node);
			return;
		}

		throw new std::exception("Missing serializer!");
	}

	void Serializers::SerializeProperty(const FieldData* pFieldData, void* data, Utils::NodeValueRef node)
	{
		if (pFieldData->Type() == ST_Array)
		{
			return GetSerializer(ST_Array)->Serialize(pFieldData->Name(), data, pFieldData->ArrayElementType(), node);
		}

		const Utils::Reflect::TypeData* pTypeData = Reflect::GetTyeData(pFieldData->ArrayElementType());
		if (pTypeData)
		{
			SerializeProperty(pFieldData->Name(), pTypeData, data, node);
			return;
		}

		PropertySerializer* pSerializer = GetSerializer(pFieldData->Type());
		if (pSerializer)
		{
			pSerializer->Serialize(pFieldData->Name(), data, pFieldData->Type(), node);
			return;
		}

		throw new std::exception("Missing serializer!");
	}

	void Serializers::DeserializeProperty(const Utils::Reflect::TypeData* pTypeData, void* data, Utils::NodeValueRef node)
	{
		PropertySerializer* pSerializer = GetSerializer(pTypeData->TypeHash());
		PropertySerializer* pInternalSerializer = GetSerializer(pTypeData->InternalTypeHash());
		if (pSerializer)
		{
			pSerializer->Deserialize(data, pTypeData->TypeHash(), node);
			return;
		}
		if (pInternalSerializer)
		{
			pInternalSerializer->Deserialize(data, pTypeData->TypeHash(), node);
			return;
		}

		throw new std::exception("Missing serializer!");
	}

	void Serializers::DeserializeProperty(const FieldData* pFieldData, void* data, Utils::NodeValueRef node)
	{
		if (pFieldData->Type() == ST_Array)
		{
			return GetSerializer(ST_Array)->Deserialize(data, pFieldData->ArrayElementType(), node);
		}

		const Utils::Reflect::TypeData* pTypeData = Reflect::GetTyeData(pFieldData->ArrayElementType());
		if (pTypeData)
		{
			DeserializeProperty(pTypeData, data, node);
			return;
		}

		PropertySerializer* pSerializer = GetSerializer(pFieldData->Type());
		if (pSerializer)
		{
			pSerializer->Deserialize(data, pFieldData->Type(), node);
			return;
		}

		throw new std::exception("Missing serializer!");
	}

	Engine* Serializers::GetEngine() const
	{
		return m_pEngine;
	}
}
