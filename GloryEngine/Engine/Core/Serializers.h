#pragma once
#include <vector>
#include <string>

#include <NodeRef.h>

namespace YAML
{
	class Node;
	class Emitter;
}

namespace Glory
{
	namespace Utils::Reflect
	{
		struct TypeData;
		struct FieldData;
	}

	class Engine;
	class PropertySerializer;

	class Serializers
	{
	public:
		Serializers(Engine* pEngine);
		virtual ~Serializers();

		template<class T>
		void RegisterSerializer()
		{
			PropertySerializer* pSerializer = new T(this);
			m_pRegisteredPropertySerializers.push_back(pSerializer);
		}

		PropertySerializer* GetSerializer(uint32_t typeHash);
		size_t GetID(PropertySerializer* pSerializer);

		void SerializeProperty(const std::string& name, const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, Utils::NodeValueRef node);
		void DeserializeProperty(std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, Utils::NodeValueRef node);

		void SerializeProperty(const std::string& name, const Utils::Reflect::TypeData* pTypeData, void* data, Utils::NodeValueRef node);
		void SerializeProperty(const Utils::Reflect::FieldData* pFieldData, void* data, Utils::NodeValueRef node);
		void DeserializeProperty(const Utils::Reflect::TypeData* pTypeData, void* data, Utils::NodeValueRef node);
		void DeserializeProperty(const Utils::Reflect::FieldData* pFieldData, void* data, Utils::NodeValueRef node);

		Engine* GetEngine() const;

	private:
		friend class PropertySerializer;
		std::vector<PropertySerializer*> m_pRegisteredPropertySerializers;
		Engine* m_pEngine;
	};
}
