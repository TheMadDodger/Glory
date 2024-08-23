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
		virtual void Serialize(const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, Utils::NodeValueRef node);
		virtual void Deserialize(std::vector<char>& buffer, size_t offset, size_t size, Utils::NodeValueRef node);
		
		virtual void Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node);
		virtual void Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node);

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
			PropertySerializer(pSerializers, ResourceTypes::GetHash<T>()) {}
		virtual ~SimpleTemplatedPropertySerializer() {}

	private:
		virtual void Serialize(const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, Utils::NodeValueRef node) override
		{
			T value;
			memcpy((void*)&value, (void*)&buffer[offset], size);
			node.Set(value);
		}

		virtual void Deserialize(std::vector<char>& buffer, size_t offset, size_t size, Utils::NodeValueRef node) override
		{
			if (!node.Exists()) return;
			T value = node.As<T>();
			memcpy((void*)&buffer[offset], (void*)&value, size);
		}

		virtual void Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override
		{
			T* value = (T*)data;
			node.Set(*value);
		}

		virtual void Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override
		{
			if (!node.Exists()) return;
			T* value = (T*)data;
			*value = node.As<T>();
		}
	};
}
