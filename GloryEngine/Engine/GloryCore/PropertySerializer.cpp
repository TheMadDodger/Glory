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

	void PropertySerializer::SerializeProperty(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out)
	{
		// If we have a custom serializer for this type we should prioritize it over serializing each field individually
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(pFieldData->Type());
		if (pSerializer)
		{
			pSerializer->Serialize(pFieldData, data, out);
			return;
		}

		// If no serializer is found we can move onto trying to serialize each field individually given the type exists in our Reflect database
		const GloryReflect::TypeData* pFieldTypeData = GloryReflect::Reflect::GetTyeData(pFieldData->Type());
		if (pFieldTypeData)
		{
			// Address needs to start at the beginning of the field, otherwise Set will write to the wrong offset
			size_t offset = pFieldData->Offset();
			void* pAddress = (void*)((char*)(data)+offset);

			out << YAML::Key << pFieldData->Name();
			out << YAML::Value << YAML::BeginMap;
			SerializeProperty(pFieldTypeData, pAddress, out);
			out << YAML::EndMap;
		}
	}

	void PropertySerializer::SerializeProperty(const GloryReflect::TypeData* pTypeData, void* data, YAML::Emitter& out)
	{
		for (size_t i = 0; i < pTypeData->FieldCount(); i++)
		{
			const GloryReflect::FieldData* pFieldData = pTypeData->GetFieldData(i);
			SerializeProperty(pFieldData, data, out);
		}
	}

	void PropertySerializer::DeserializeProperty(std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, YAML::Node& object)
	{
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(typeHash);
		if (pSerializer == nullptr) return;
		pSerializer->Deserialize(buffer, offset, size, object);
	}

	void PropertySerializer::DeserializeProperty(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object)
	{
		// If we have a custom serializer for this type we should prioritize it over deserializing each field individually
		PropertySerializer* pSerializer = PropertySerializer::GetSerializer(pFieldData->Type());
		if (pSerializer)
		{
			pSerializer->Deserialize(pFieldData, data, object);
			return;
		}

		// If no serializer is found we can move onto trying to deserialize each field individually given the type exists in our Reflect database
		const GloryReflect::TypeData* pFieldTypeData = GloryReflect::Reflect::GetTyeData(pFieldData->Type());
		if (pFieldTypeData)
		{
			// Address needs to start at the beginning of the field, otherwise Get will read from the wrong offset
			size_t offset = pFieldData->Offset();
			void* pAddress = (void*)((char*)(data)+offset);
			DeserializeProperty(pFieldTypeData, pAddress, object);
		}
	}

	void PropertySerializer::DeserializeProperty(const GloryReflect::TypeData* pTypeData, void* data, YAML::Node& object)
	{
		for (size_t i = 0; i < pTypeData->FieldCount(); i++)
		{
			const GloryReflect::FieldData* pFieldData = pTypeData->GetFieldData(i);
			YAML::Node nextNode = object[pFieldData->Name()];
			if (!nextNode.IsDefined())
			{
				const char* name = pFieldData->Name();
				size_t length = strlen(name);
				std::string legacyConversion = "";
				for (size_t i = 0; i < length; i++)
				{
					char c = name[i];
					if (c == '\"' || c == ' ') continue;
					legacyConversion.push_back(c);
				}
				nextNode = object[legacyConversion];
				if (!nextNode.IsDefined()) continue;
			}
			DeserializeProperty(pFieldData, data, nextNode);
		}
	}

	uint32_t PropertySerializer::GetSerializedTypeHash() const
	{
		return m_TypeHash;
	}

	void PropertySerializer::Serialize(const std::string&, const std::vector<char>&, uint32_t, size_t, size_t, YAML::Emitter&) {}

	void PropertySerializer::Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out) {}

	void PropertySerializer::Deserialize(std::vector<char>&, size_t, size_t, YAML::Node&) {}

	void PropertySerializer::Deserialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object) {}

	void PropertySerializer::Cleanup()
	{
		std::for_each(PROPERTY_SERIALIZERS.begin(), PROPERTY_SERIALIZERS.end(), [](PropertySerializer* pSerializer) { delete pSerializer; });
		PROPERTY_SERIALIZERS.clear();
	}
}
