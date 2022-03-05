#pragma once
#include <yaml-cpp/yaml.h>
#include "SerializedProperty.h"
#include "YAML_GLM.h"
#include "GLORY_YAML.h"
#define STANDARD_SERIALIZER(x) PropertySerializer::RegisterSerializer<SimpleTemplatedPropertySerializer<x>>()

namespace Glory
{

	class PropertySerializer
	{
	public:
		template<class T>
		static void RegisterSerializer()
		{
			PropertySerializer* pSerializer = new T();
			m_pRegisteredSerializers.push_back(pSerializer);
		}

		//static PropertySerializer* GetSerializer(Object* pObject);
		static PropertySerializer* GetSerializer(size_t typeHash);
		static PropertySerializer* GetSerializer(const SerializedProperty* serializedProperty);
		static size_t GetID(PropertySerializer* pSerializer);

		static void SerializeProperty(const SerializedProperty* serializedProperty, YAML::Emitter& out);
		static void DeserializeProperty(const SerializedProperty* serializedProperty, YAML::Node& object);
		static void DeserializeProperty(std::any& out, size_t typeHash, YAML::Node& object);

		virtual size_t GetSerializedTypeHash() const;

	protected:
		PropertySerializer(size_t typeHash);
		virtual ~PropertySerializer();

	protected:
		virtual void Serialize(const SerializedProperty* serializedProperty, YAML::Emitter& out) = 0;
		virtual void Deserialize(const SerializedProperty* serializedProperty, YAML::Node& object) = 0;
		virtual void Deserialize(std::any& out, YAML::Node& object) = 0;

	private:
		friend class Engine;
		static void Cleanup();

	private:
		static std::vector<PropertySerializer*> m_pRegisteredSerializers;
		size_t m_TypeHash;
	};

	template<typename T>
	class SimpleTemplatedPropertySerializer : public PropertySerializer
	{
	public:
		SimpleTemplatedPropertySerializer() : PropertySerializer(ResourceType::GetHash<T>()) {}
		virtual ~SimpleTemplatedPropertySerializer() {}

	private:
		virtual void Serialize(const SerializedProperty* serializedProperty, YAML::Emitter& out) override
		{
			T value = *(T*)serializedProperty->MemberPointer();
			out << YAML::Key << serializedProperty->Name();
			out << YAML::Value << value;
		}

		virtual void Deserialize(const SerializedProperty* serializedProperty, YAML::Node& object) override
		{
			if (!object.IsDefined()) return;
			T* pMember = (T*)serializedProperty->MemberPointer();
			T data = object.as<T>();
			*pMember = data;
		}

		virtual void Deserialize(std::any& out, YAML::Node& object) override
		{
			if (!object.IsDefined()) return;
			out = object.as<T>();
		}
	};
}
