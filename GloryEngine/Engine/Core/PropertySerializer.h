#pragma once
#include "YAML_GLM.h"
#include "GLORY_YAML.h"
#include "SerializedTypes.h"
#include "Serializers.h"

#include <yaml-cpp/yaml.h>
#include <Reflection.h>

namespace Glory
{
	class PropertySerializer
	{
	public:
		virtual uint32_t GetSerializedTypeHash() const;

	protected:
		PropertySerializer(Serializers* pSerializers, uint32_t typeHash);
		virtual ~PropertySerializer();

	public:
		virtual void Serialize(const std::string& name, const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, YAML::Emitter& out);
		virtual void Deserialize(std::vector<char>& buffer, size_t offset, size_t size, YAML::Node& object);
		
		virtual void Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out);
		virtual void Deserialize(void* data, uint32_t typeHash, YAML::Node& object);

	protected:
		Serializers* m_pSerializers;

	private:
		friend class Engine;
		friend class Serializers;

	private:
		uint32_t m_TypeHash;
	};

	template<typename T>
	class SimpleTemplatedPropertySerializer : public PropertySerializer
	{
	public:
		SimpleTemplatedPropertySerializer(Serializers* pSerializers):
			PropertySerializer(pSerializers, ResourceType::GetHash<T>()) {}
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
