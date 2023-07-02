#pragma once
#include <yaml-cpp/yaml.h>
#include "YAML_GLM.h"
#include "GLORY_YAML.h"
#include "GloryContext.h"
#include "Serializer.h"
#include <Reflection.h>

#ifndef PROPERTY_SERIALIZERS
#define PROPERTY_SERIALIZERS Glory::GloryContext::GetSerializers()->m_pRegisteredPropertySerializers
#endif
#ifndef STANDARD_SERIALIZER
#define STANDARD_SERIALIZER(x) PropertySerializer::RegisterSerializer<SimpleTemplatedPropertySerializer<x>>()
#endif

namespace Glory
{

	class PropertySerializer
	{
	public:
		template<class T>
		static void RegisterSerializer()
		{
			PropertySerializer* pSerializer = new T();
			PROPERTY_SERIALIZERS.push_back(pSerializer);
		}

		//static PropertySerializer* GetSerializer(Object* pObject);
		static PropertySerializer* GetSerializer(uint32_t typeHash);
		static size_t GetID(PropertySerializer* pSerializer);

		static void SerializeProperty(const std::string& name, const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, YAML::Emitter& out);
		static void DeserializeProperty(std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, YAML::Node& object);

		static void SerializeProperty(const std::string& name, const TypeData* pTypeData, void* data, YAML::Emitter& out);
		static void SerializeProperty(const FieldData* pFieldData, void* data, YAML::Emitter& out);
		static void DeserializeProperty(const TypeData* pTypeData, void* data, YAML::Node& object);
		static void DeserializeProperty(const FieldData* pFieldData, void* data, YAML::Node& object);

		virtual uint32_t GetSerializedTypeHash() const;

	protected:
		PropertySerializer(uint32_t typeHash);
		virtual ~PropertySerializer();

	public:
		virtual void Serialize(const std::string& name, const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, YAML::Emitter& out);
		virtual void Deserialize(std::vector<char>& buffer, size_t offset, size_t size, YAML::Node& object);
		
		virtual void Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out);
		virtual void Deserialize(void* data, uint32_t typeHash, YAML::Node& object);


	private:
		friend class Engine;
		static void Cleanup();

	private:
		uint32_t m_TypeHash;
	};

	template<typename T>
	class SimpleTemplatedPropertySerializer : public PropertySerializer
	{
	public:
		SimpleTemplatedPropertySerializer() : PropertySerializer(ResourceType::GetHash<T>()) {}
		virtual ~SimpleTemplatedPropertySerializer() {}

	private:
		virtual void Serialize(const std::string& name, const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, YAML::Emitter& out) override
		{
			T value;
			memcpy((void*)&value, (void*)&buffer[offset], size);
			out << YAML::Key << name;
			out << YAML::Value << value;
		}

		virtual void Deserialize(std::vector<char>& buffer, size_t offset, size_t size, YAML::Node& object) override
		{
			if (!object.IsDefined()) return;
			T value = object.as<T>();
			memcpy((void*)&buffer[offset], (void*)&value, size);
		}

		virtual void Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out) override
		{
			T* value = (T*)data;
			/* Nameless properties can come from array elements */
			if (name.empty())
			{
				out << *value;
				return;
			}

			out << YAML::Key << name;
			out << YAML::Value << *value;
		}

		virtual void Deserialize(void* data, uint32_t typeHash, YAML::Node& object) override
		{
			if (!object.IsDefined()) return;
			T* value = (T*)data;
			*value = object.as<T>();
		}
	};
}
