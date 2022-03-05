#include "EntitySceneObjectSerializer.h"
#include "EntityComponentObject.h"
#include "EntitySceneScenesModule.h"
#include <PropertySerializer.h>
#include "Components.h"

namespace Glory
{
	EntitySceneObjectSerializer::EntitySceneObjectSerializer()
	{
	}

	EntitySceneObjectSerializer::~EntitySceneObjectSerializer()
	{
	}

	void EntitySceneObjectSerializer::Serialize(EntitySceneObject* pObject, YAML::Emitter& out)
	{
		out << YAML::Key << "Name";
		out << YAML::Value << pObject->Name();
		out << YAML::Key << "Components";
		out << YAML::Value << YAML::BeginSeq;
		pObject->GetEntityHandle().ForEachComponent([&](Registry* pRegistry, EntityID entityID, EntityComponentData* pComponentData)
		{
			EntityComponentObject* pComponentObject = new EntityComponentObject(pComponentData, pRegistry);
			out << YAML::BeginMap;
			out << YAML::Key << "UUID";
			out << YAML::Value << pComponentObject->GetComponentUUID();
			out << YAML::Key << "TypeHash";
			std::type_index type = pComponentData->GetType();
			out << YAML::Value << ResourceType::GetHash(type);

			std::vector<SerializedProperty*> properties;
			pRegistry->GetSystems()->AcquireSerializedProperties(pComponentData, properties);
			for (size_t i = 0; i < properties.size(); i++)
			{
				PropertySerializer::SerializeProperty(properties[i], out);
			}
			out << YAML::EndMap;

			//Serializer::SerializeObject(pComponentObject, out);
			delete pComponentObject;
		});
		out << YAML::EndSeq;
	}

	Object* EntitySceneObjectSerializer::Deserialize(Object* pParent, YAML::Node& object)
	{
		YAML::Node node;
		std::string name;
		UUID uuid;
		YAML_READ(object, node, Name, name, std::string);
		YAML_READ(object, node, UUID, uuid, uint64_t);

		EntityScene* pScene = (EntityScene*)pParent;
		EntitySceneObject* pObject = (EntitySceneObject*)pScene->CreateEmptyObject(name, uuid);
		node = object["Components"];

		size_t currentComponentIndex = 0;

		for (size_t i = 0; i < node.size(); i++)
		{
			YAML::Node nextObject = node[i];
			YAML::Node subNode;
			UUID compUUID;
			size_t typeHash = 0;
			YAML_READ(nextObject, subNode, UUID, compUUID, uint64_t);
			YAML_READ(nextObject, subNode, TypeHash, typeHash, size_t);

			Entity entityHandle = pObject->GetEntityHandle();
			EntityID entity = entityHandle.GetEntityID();

			pScene->GetRegistry()->GetSystems()->CreateComponent(entity, typeHash, compUUID);
			EntityComponentData* pComponentData = pScene->GetRegistry()->GetEntityComponentDataAt(entity, currentComponentIndex);
			if (!pComponentData) continue;
			std::vector<SerializedProperty*> properties;
			pScene->GetRegistry()->GetSystems()->AcquireSerializedProperties(pComponentData, properties);

			for (size_t i = 0; i < properties.size(); i++)
			{
				const SerializedProperty* serializedProperty = properties[i];
				YAML::Node dataNode = nextObject[serializedProperty->Name()];
				PropertySerializer::DeserializeProperty(serializedProperty, dataNode);
			}
			++currentComponentIndex;

			//Serializer::DeserializeObject(pObject, nextObject);
		}

		return pScene;
	}
}
