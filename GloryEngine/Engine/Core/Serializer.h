#pragma once
#include <typeindex>
#include <yaml-cpp/yaml.h>
#include "YAML_GLM.h"
#include "GloryContext.h"
#include "Object.h"

#ifndef SERIALIZERS
#define SERIALIZERS Glory::GloryContext::GetSerializers()->m_pRegisteredSerializers
#endif

namespace Glory
{
	class Serializer
	{
	public:
		enum Flags
		{
			GenerateNewUUIDs = 1,
			IgnorePrefabs = 2,
		};

	public:
		template<class T>
		static void RegisterSerializer()
		{
			Serializer* pSerializer = new T();
			SERIALIZERS.push_back(pSerializer);
		}

		static Serializer* GetSerializer(Object* pObject);
		static Serializer* GetSerializer(uint32_t typeHash);
		static size_t GetID(Serializer* pSerializer);
		static void SerializeObject(Object* pObject, YAML::Emitter& out);

		template<class T>
		static T* DeserializeObject(YAML::Node& object, UUID uuid)
		{
			return (T*)DeserializeObject(object, uuid);
		}

		static Object* DeserializeObject(YAML::Node& object, UUID uuid);

		template<class T>
		static T* DeserializeObjectOfType(YAML::Node& object, UUID uuid, const std::string& name = "")
		{
			return (T*)DeserializeObjectOfType(typeid(T), object, uuid, name);
		}

		template<class T>
		static T* DeserializeObject(Object* pParent, UUID uuid, YAML::Node& object)
		{
			return (T*)DeserializeObject(pParent, object, uuid);
		}

		static Object* DeserializeObject(Object* pParent, YAML::Node& object, UUID uuid, Flags flags = Flags(0));
		static Object* DeserializeObjectOfType(std::type_index type, YAML::Node& object, UUID uuid, const std::string& name = "");

		virtual std::type_index GetSerializedType() = 0;

		virtual void Serialize(Object* pObject, YAML::Emitter& out) = 0;
		virtual Object* Deserialize(Object* pParent, YAML::Node& object, UUID uuid, const std::string& name = "", Flags flags = Flags(0)) = 0;

		static void ClearUUIDRemapCache();
		static void SetUUIDRemap(UUID oldUUID, UUID newUUID);

	protected:
		Serializer();
		virtual ~Serializer();

	private:
		friend class Engine;
		static void Cleanup();

	private:
	};

	class PropertySerializer;

	class Serializers
	{
	public:
		Serializers();
		virtual ~Serializers();

	private:
		friend class Serializer;
		friend class PropertySerializer;
		std::vector<Serializer*> m_pRegisteredSerializers;
		std::vector<PropertySerializer*> m_pRegisteredPropertySerializers;
	};

	template<class TObject>
	class SerializerTemplate : public Serializer
	{
	public:
		virtual std::type_index GetSerializedType() override
		{
			return typeid(TObject);
		}

	private:

		virtual void Serialize(Object* pObject, YAML::Emitter& out) override
		{
			Serialize((TObject*)pObject, out);
		}

		virtual void Serialize(TObject* pObject, YAML::Emitter& out) = 0;

		// This function ensures that T is of type Object by causing a compiler error if it isn't
		void CompilerTest();
	};
}
