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

	void EntitySceneObjectSerializer::SerializeComponent(Registry* pRegistry, EntityComponentData* pComponentData, YAML::Emitter& out)
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
		delete pComponentObject;
	}

	void EntitySceneObjectSerializer::DeserializeComponent(EntityScene* pScene, EntitySceneObject* pObject, size_t componentIndex, YAML::Node& object)
	{
		YAML::Node subNode;
		UUID compUUID;
		size_t typeHash = 0;
		YAML_READ(object, subNode, UUID, compUUID, uint64_t);
		YAML_READ(object, subNode, TypeHash, typeHash, size_t);

		Entity entityHandle = pObject->GetEntityHandle();
		EntityID entity = entityHandle.GetEntityID();

		Registry* pRegistry = pScene->GetRegistry();
		EntitySystems* pSystems = pRegistry->GetSystems();

		size_t newComponentIndex = pRegistry->ComponentCount(entity);

		pSystems->CreateComponent(entity, typeHash, compUUID);
		if (newComponentIndex != componentIndex)
			pRegistry->ChangeComponentIndex(entity, newComponentIndex, componentIndex);

		EntityComponentData* pComponentData = pRegistry->GetEntityComponentDataAt(entity, componentIndex);

		if (!pComponentData) return;
		std::vector<SerializedProperty*> properties;
		pSystems->AcquireSerializedProperties(pComponentData, properties);

		for (size_t i = 0; i < properties.size(); i++)
		{
			const SerializedProperty* serializedProperty = properties[i];
			YAML::Node dataNode = object[serializedProperty->Name()];
			PropertySerializer::DeserializeProperty(serializedProperty, dataNode);
		}
	}

	void EntitySceneObjectSerializer::Serialize(EntitySceneObject* pObject, YAML::Emitter& out)
	{
		SceneObject* pParent = pObject->GetParent();

		out << YAML::Key << "Name";
		out << YAML::Value << pObject->Name();
		out << YAML::Key << "UUID";
		out << YAML::Value << pObject->GetUUID();
		out << YAML::Key << "ParentUUID";
		out << YAML::Value << (pParent ? pParent->GetUUID() : 0);
		out << YAML::Key << "Components";
		out << YAML::Value << YAML::BeginSeq;
		pObject->GetEntityHandle().ForEachComponent([&](Registry* pRegistry, EntityID entityID, EntityComponentData* pComponentData)
		{
			SerializeComponent(pRegistry, pComponentData, out);
		});
		out << YAML::EndSeq;
	}

	Object* EntitySceneObjectSerializer::Deserialize(Object* pParent, YAML::Node& object, const std::string&)
	{
		YAML::Node node;
		std::string name;
		UUID uuid;
		UUID parentUuid;
		YAML_READ(object, node, Name, name, std::string);
		YAML_READ(object, node, UUID, uuid, uint64_t);
		YAML_READ(object, node, ParentUUID, parentUuid, uint64_t);

		EntityScene* pScene = (EntityScene*)pParent;
		EntitySceneObject* pObject = (EntitySceneObject*)pScene->CreateEmptyObject(name, uuid);
		node = object["Components"];

		if (parentUuid != NULL)
		{
			SceneObject* pParent = pScene->FindSceneObject(parentUuid);
			if (pParent == nullptr) pScene->DelayedSetParent(pObject, parentUuid);
			else pObject->SetParent(pParent);
		}

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
		}

		return pScene;
	}
}
