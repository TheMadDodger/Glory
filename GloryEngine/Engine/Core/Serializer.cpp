#include <algorithm>
#include "Serializer.h"
#include "ResourceType.h"
#include "GScene.h"
#include "SerializedTypes.h"

namespace Glory
{
	/* This ensures legacy support by rerouting an old hash to a new one */
	const std::map<uint32_t, uint32_t> LegacyRehasher = {
		{ 3536807891, ResourceType::GetHash<GScene>() },
		{ 1002038608, SerializedType::ST_Object }
	};

	Serializer* Serializer::GetSerializer(Object* pObject)
	{
		for (size_t i = 0; i < pObject->TypeCount(); i++)
		{
			std::type_index type = typeid(Object);
			if (!pObject->GetType(i, type)) continue;

			auto it = std::find_if(SERIALIZERS.begin(), SERIALIZERS.end(), [&](Serializer* pSerializer)
			{
				return pSerializer->GetSerializedType() == type;
			});

			if (it == SERIALIZERS.end()) continue;
			Serializer* pSerializer = *it;
			return pSerializer;
		}

		return nullptr;
	}

	Serializer* Serializer::GetSerializer(uint32_t typeHash)
	{
		const auto itor = LegacyRehasher.find(typeHash);
		typeHash = itor != LegacyRehasher.end() ? itor->second : typeHash;

		for (size_t i = 0; i < SERIALIZERS.size(); i++)
		{
			Serializer* pSerializer = SERIALIZERS[i];
			std::type_index type = pSerializer->GetSerializedType();
			uint32_t hash = ResourceType::GetHash(type);
			if (hash == typeHash) return pSerializer;
		}

		return nullptr;

		//auto it = std::find_if(m_pRegisteredSerializers.begin(), m_pRegisteredSerializers.end(), [&](Serializer* pSerializer)
		//{
		//	std::type_index type = pSerializer->GetSerializedType();
		//	uint32_t hash = ResourceType::GetHash(type);
		//	return hash == typeHash;
		//});
		//
		//if (it == m_pRegisteredSerializers.end()) return nullptr;
		//Serializer* pSerializer = *it;
		//return pSerializer;
	}

	size_t Serializer::GetID(Serializer* pSerializer)
	{
		for (size_t i = 0; i < SERIALIZERS.size(); i++)
		{
			if (SERIALIZERS[i] == pSerializer) return i;
		}
		return 0;
	}

	void Serializer::SerializeObject(Object* pObject, YAML::Emitter& out)
	{
		Serializer* pSerializer = Serializer::GetSerializer(pObject);
		if (pSerializer == nullptr) return;

		out << YAML::BeginMap;
		//out << YAML::Key << "UUID";
		//out << YAML::Value << pObject->GetUUID();
		out << YAML::Key << "TypeHash";
		std::type_index type = typeid(Object);
		pObject->GetType(0, type);
		out << YAML::Value << ResourceType::GetHash(type);
		pSerializer->Serialize(pObject, out);
		out << YAML::EndMap;
	}

	Object* Serializer::DeserializeObject(YAML::Node& object, UUID uuid)
	{
		return DeserializeObject(nullptr, object, uuid);
	}

	Object* Serializer::DeserializeObject(Object* pParent, YAML::Node& object, UUID uuid, Flags flags)
	{
		YAML::Node node;
		uint32_t typeHash = 0;
		YAML_READ(object, node, TypeHash, typeHash, uint32_t);

		Serializer* pSerializer = GetSerializer(typeHash);
		if (pSerializer == nullptr) return nullptr;
		return pSerializer->Deserialize(pParent, object, uuid, "", flags);
	}

	Object* Serializer::DeserializeObjectOfType(std::type_index type, YAML::Node& object, UUID uuid, const std::string& name)
	{
		uint32_t typeHash = ResourceType::GetHash(type);
		Serializer* pSerializer = GetSerializer(typeHash);
		if (pSerializer == nullptr) return nullptr;
		return pSerializer->Deserialize(nullptr, object, uuid, name);
	}

	void Serializer::ClearUUIDRemapCache()
	{
		GloryContext::GetContext()->m_UUIDRemapper.Reset();
	}

	void Serializer::SetUUIDRemap(UUID oldUUID, UUID newUUID)
	{
		GloryContext::GetContext()->m_UUIDRemapper.EnforceRemap(oldUUID, newUUID);
	}

	Serializer::Serializer()
	{
	}

	Serializer::~Serializer()
	{
	}

	void Serializer::Cleanup()
	{
		std::for_each(SERIALIZERS.begin(), SERIALIZERS.end(), [](Serializer* pSerializer) { delete pSerializer; });
		SERIALIZERS.clear();
	}

	template<class TObject>
	inline void SerializerTemplate<TObject>::CompilerTest()
	{
		Object* pObject = new TObject();
	}
	Serializers::Serializers()
	{
	}
	Serializers::~Serializers()
	{
	}
}
