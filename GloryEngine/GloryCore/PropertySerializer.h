#pragma once
#include <yaml-cpp/yaml.h>
#include "SerializedProperty.h"
#include "YAML_GLM.h"
#include "GLORY_YAML.h"
#include "GloryContext.h"
#include "Serializer.h"
#include <Reflection.h>

#define SERIALIZERS Glory::GloryContext::GetSerializers()->m_pRegisteredPropertySerializers
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
			SERIALIZERS.push_back(pSerializer);
		}

		//static PropertySerializer* GetSerializer(Object* pObject);
		static PropertySerializer* GetSerializer(size_t typeHash);
		static PropertySerializer* GetSerializer(const SerializedProperty* serializedProperty);
		static size_t GetID(PropertySerializer* pSerializer);

		static void SerializeProperty(const SerializedProperty* serializedProperty, YAML::Emitter& out);
		static void SerializeProperty(const std::string& name, const std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, YAML::Emitter& out);
		static void SerializeProperty(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out);
		static void SerializeProperty(const GloryReflect::TypeData* pTypeData, void* data, YAML::Emitter& out);
		static void DeserializeProperty(const SerializedProperty* serializedProperty, YAML::Node& object);
		static void DeserializeProperty(std::any& out, size_t typeHash, YAML::Node& object);
		static void DeserializeProperty(std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, YAML::Node& object);
		static void DeserializeProperty(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object);
		static void DeserializeProperty(const GloryReflect::TypeData* pTypeData, void* data, YAML::Node& object);

		virtual size_t GetSerializedTypeHash() const;

	protected:
		PropertySerializer(size_t typeHash);
		virtual ~PropertySerializer();

	protected:
		virtual void Serialize(const SerializedProperty* serializedProperty, YAML::Emitter& out) = 0;
		virtual void Serialize(const std::string& name, const std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, YAML::Emitter& out);
		virtual void Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out);
		virtual void Deserialize(const SerializedProperty* serializedProperty, YAML::Node& object) = 0;
		virtual void Deserialize(std::any& out, YAML::Node& object) = 0;
		virtual void Deserialize(std::vector<char>& buffer, size_t offset, size_t size, YAML::Node& object);
		virtual void Deserialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object);

	private:
		friend class Engine;
		static void Cleanup();

	private:
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

		virtual void Serialize(const std::string& name, const std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, YAML::Emitter& out) override
		{
			T value;
			memcpy((void*)&value, (void*)&buffer[offset], size);
			out << YAML::Key << name;
			out << YAML::Value << value;
		}

		virtual void Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out) override
		{
			T value;
			pFieldData->Get(data, &value);
			out << YAML::Key << pFieldData->Name();
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

		virtual void Deserialize(std::vector<char>& buffer, size_t offset, size_t size, YAML::Node& object) override
		{
			if (!object.IsDefined()) return;
			T value = object.as<T>();
			memcpy((void*)&buffer[offset], (void*)&value, size);
		}

		virtual void Deserialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object) override
		{
			if (!object.IsDefined()) return;
			T value = object.as<T>();
			pFieldData->Set(data, &value);
		}
	};
}
