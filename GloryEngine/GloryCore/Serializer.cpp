#include <algorithm>
#include "Serializer.h"
#include "ResourceType.h"

namespace Glory
{
	std::vector<Serializer*> Serializer::m_pRegisteredSerializers = std::vector<Serializer*>();

	Serializer* Serializer::GetSerializer(Object* pObject)
	{
		for (size_t i = 0; i < pObject->TypeCount(); i++)
		{
			std::type_index type = typeid(Object);
			if (!pObject->GetType(i, type)) continue;

			auto it = std::find_if(m_pRegisteredSerializers.begin(), m_pRegisteredSerializers.end(), [&](Serializer* pSerializer)
			{
				return pSerializer->GetSerializedType() == type;
			});

			if (it == m_pRegisteredSerializers.end()) continue;
			Serializer* pSerializer = *it;
			return pSerializer;
		}

		return nullptr;
	}

	Serializer* Serializer::GetSerializer(size_t typeHash)
	{
		for (size_t i = 0; i < m_pRegisteredSerializers.size(); i++)
		{
			Serializer* pSerializer = m_pRegisteredSerializers[i];
			std::type_index type = pSerializer->GetSerializedType();
			size_t hash = ResourceType::GetHash(type);
			if (hash == typeHash) return pSerializer;
		}

		return nullptr;

		//auto it = std::find_if(m_pRegisteredSerializers.begin(), m_pRegisteredSerializers.end(), [&](Serializer* pSerializer)
		//{
		//	std::type_index type = pSerializer->GetSerializedType();
		//	size_t hash = ResourceType::GetHash(type);
		//	return hash == typeHash;
		//});
		//
		//if (it == m_pRegisteredSerializers.end()) return nullptr;
		//Serializer* pSerializer = *it;
		//return pSerializer;
	}

	size_t Serializer::GetID(Serializer* pSerializer)
	{
		for (size_t i = 0; i < m_pRegisteredSerializers.size(); i++)
		{
			if (m_pRegisteredSerializers[i] == pSerializer) return i;
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

	Object* Serializer::DeserializeObject(YAML::Node& object)
	{
		return DeserializeObject(nullptr, object);
	}

	Object* Serializer::DeserializeObject(Object* pParent, YAML::Node& object)
	{
		YAML::Node node;
		size_t typeHash = 0;
		YAML_READ(object, node, TypeHash, typeHash, size_t);

		Serializer* pSerializer = GetSerializer(typeHash);
		if (pSerializer == nullptr) return nullptr;
		return pSerializer->Deserialize(pParent, object);
	}

	Object* Serializer::DeserializeObjectOfType(std::type_index type, YAML::Node& object, const std::string& name)
	{
		size_t typeHash = ResourceType::GetHash(type);
		Serializer* pSerializer = GetSerializer(typeHash);
		if (pSerializer == nullptr) return nullptr;
		return pSerializer->Deserialize(nullptr, object, name);
	}

	Serializer::Serializer()
	{
	}

	Serializer::~Serializer()
	{
	}

	void Serializer::Cleanup()
	{
		std::for_each(m_pRegisteredSerializers.begin(), m_pRegisteredSerializers.end(), [](Serializer* pSerializer) { delete pSerializer; });
		m_pRegisteredSerializers.clear();
	}

	template<class TObject>
	inline void SerializerTemplate<TObject>::CompilerTest()
	{
		Object* pObject = new TObject();
	}
}
