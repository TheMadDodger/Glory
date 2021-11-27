#pragma once
#include "Object.h"
#include <typeindex>
#include <yaml-cpp/yaml.h>
#include "YAML_GLM.h"

namespace Glory
{
	class Serializer
	{
	public:
		template<class T>
		static void RegisterSerializer()
		{
			Serializer* pSerializer = new T();
			m_pRegisteredSerializers.push_back(pSerializer);
		}

		static Serializer* GetSerializer(Object* pObject);
		static Serializer* GetSerializer(size_t typeHash);
		static size_t GetID(Serializer* pSerializer);
		static void SerializeObject(Object* pObject, YAML::Emitter& out);

		template<class T>
		static T* DeserializeObject(YAML::Node& object)
		{
			return (T*)DeserializeObject(object);
		}

		static Object* DeserializeObject(YAML::Node& object);

		template<class T>
		static T* DeserializeObject(Object* pParent, YAML::Node& object)
		{
			return (T*)DeserializeObject(pParent, object);
		}

		static Object* DeserializeObject(Object* pParent, YAML::Node& object);

		virtual const std::type_index& GetSerializedType() = 0;

	protected:
		Serializer();
		virtual ~Serializer();

	protected:
		virtual void Serialize(Object* pObject, YAML::Emitter& out) = 0;
		virtual Object* Deserialize(Object* pParent, YAML::Node& object) = 0;

	private:
		friend class Engine;
		static void Cleanup();

	private:
		static std::vector<Serializer*> m_pRegisteredSerializers;
	};

	template<class TObject>
	class SerializerTemplate : public Serializer
	{
	public:
		virtual const std::type_index& GetSerializedType() override
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
