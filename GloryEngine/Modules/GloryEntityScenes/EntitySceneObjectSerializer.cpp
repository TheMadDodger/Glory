#include "EntitySceneObjectSerializer.h"
#include "EntityComponentObject.h"
#include "EntitySceneScenesModule.h"
#include "Components.h"
#include "EntityPrefabData.h"

#include <PropertySerializer.h>
#include <NodeRef.h>

namespace Glory
{
	EntitySceneObjectSerializer::EntitySceneObjectSerializer()
	{
	}

	EntitySceneObjectSerializer::~EntitySceneObjectSerializer()
	{
	}

	void EntitySceneObjectSerializer::SerializeComponent(Glory::Utils::ECS::EntityID entityID, Glory::Utils::ECS::EntityRegistry* pRegistry, UUID componentUUID, void* pAddress, const TypeData* pTypeData, YAML::Emitter& out)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "UUID";
		out << YAML::Value << componentUUID;
		out << YAML::Key << "TypeName";
		out << YAML::Value << pTypeData->TypeName();
		out << YAML::Key << "TypeHash";
		out << YAML::Value << pTypeData->TypeHash();
		PropertySerializer::SerializeProperty("Properties", pTypeData, pAddress, out);
		out << YAML::EndMap;
	}

	void EntitySceneObjectSerializer::DeserializeComponent(EntityScene* pScene, EntitySceneObject* pObject, size_t componentIndex, YAML::Node& object)
	{
		YAML::Node nextObject = object;
		YAML::Node subNode;
		UUID compUUID;
		uint32_t typeHash = 0;
		std::string typeName = "";
		YAML_READ(nextObject, subNode, UUID, compUUID, uint64_t);
		YAML_READ(nextObject, subNode, TypeName, typeName, std::string);
		YAML_READ(nextObject, subNode, TypeHash, typeHash, uint32_t);

		Entity entityHandle = pObject->GetEntityHandle();
		EntityID entity = entityHandle.GetEntityID();
		EntityRegistry* pRegistry = pScene->GetRegistry();

		void* pComponentAddress = pRegistry->CreateComponent(entity, typeHash, compUUID);
		EntityView* pEntityView = pRegistry->GetEntityView(entity);
		pEntityView->SetComponentIndex(pEntityView->ComponentCount() - 1, componentIndex);

		const TypeData* pTypeData = Reflect::GetTyeData(typeHash);
		PropertySerializer::DeserializeProperty(pTypeData, pComponentAddress, nextObject["Properties"]);

		pRegistry->GetTypeView(typeHash)->Invoke(InvocationType::OnValidate, pRegistry, entity, pComponentAddress);
	}

	void WriteIDSMap(EntitySceneObject* pChild, YAML::Emitter& out)
	{

	}

	void EntitySceneObjectSerializer::Serialize(EntitySceneObject* pObject, YAML::Emitter& out)
	{
		SceneObject* pParent = pObject->GetParent();

		GScene* pScene = pObject->GetScene();

		out << YAML::Key << "Name";
		out << YAML::Value << pObject->Name();
		out << YAML::Key << "UUID";
		out << YAML::Value << pObject->GetUUID();
		out << YAML::Key << "Active";
		out << YAML::Value << pObject->IsActiveSelf();
		out << YAML::Key << "ParentUUID";
		out << YAML::Value << (pParent ? pParent->GetUUID() : 0);

		const UUID prefabID = pScene->Prefab(pObject->GetUUID());
		if (prefabID)
		{
			/* Serialize the prefab ID instead */
			out << YAML::Key << "PrefabID";
			out << YAML::Value << prefabID;

			/* TODO: Serialize ID remapping */

			/* TODO: Serialize overrides */
			return;
		}

		out << YAML::Key << "Components";
		out << YAML::Value << YAML::BeginSeq;
		Entity entity = pObject->GetEntityHandle();
		EntityRegistry* pRegistry = entity.GetScene()->GetRegistry();
		EntityView* pEntityView = entity.GetEntityView();
		for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
		{
			UUID componentUUID = pEntityView->ComponentUUIDAt(i);
			uint32_t typeHash = pEntityView->ComponentTypeAt(i);
			BaseTypeView* pTypeView = pRegistry->GetTypeView(typeHash);
			void* pAddress = pTypeView->GetComponentAddress(entity.GetEntityID());
			const TypeData* pTypeData = Reflect::GetTyeData(typeHash);
			SerializeComponent(entity.GetEntityID(), pRegistry, componentUUID, pAddress, pTypeData, out);
		}
		out << YAML::EndSeq;
	}

	Object* EntitySceneObjectSerializer::Deserialize(Object* pParent, YAML::Node& object, const std::string&, Flags flags)
	{
		EntityScene* pScene = (EntityScene*)pParent;

		std::map<UUID, UUID>& uuidRemapper = GloryContext::GetContext()->m_UUIDRemapper;

		YAML::Node node;
		std::string name;
		UUID uuid;
		UUID parentUuid;
		bool active = true;
		YAML_READ(object, node, Name, name, std::string);
		YAML_READ(object, node, UUID, uuid, uint64_t);
		YAML_READ(object, node, Active, active, bool);
		YAML_READ(object, node, ParentUUID, parentUuid, uint64_t);

		NodeRef nodeRef{ object };
		NodeValueRef prefabIDRef = nodeRef["PrefabID"];
		if (prefabIDRef.Exists())
		{
			const UUID prefabID = prefabIDRef.As<uint64_t>();
			EntityPrefabData* pPrefab = AssetManager::GetAssetImmediate<EntityPrefabData>(prefabID);
			if (pPrefab)
			{
				SceneObject* pInstantiatedPrefab = pScene->InstantiatePrefab(uuid, pPrefab);

				/* TODO: Deserialize overrides */
				return pInstantiatedPrefab;
			}
		}

		if (flags & Serializer::Flags::GenerateNewUUIDs)
		{
			if (uuidRemapper.find(uuid) != uuidRemapper.end())
			{
				/* Use existing newly generated UUID */
				uuid = uuidRemapper.at(uuid);
			}
			else
			{
				/* Generate new UUID */
				UUID newUUID = UUID();
				uuidRemapper.emplace(uuid, newUUID);
				uuid = newUUID;
			}

			/* Remap parent */
			if (parentUuid)
			{
				if (uuidRemapper.find(parentUuid) == uuidRemapper.end())
				{
					/* Generate new UUID for parent */
					UUID newParentUuid = UUID();
					uuidRemapper.emplace(parentUuid, newParentUuid);
				}

				parentUuid = uuidRemapper.at(parentUuid);
			}
		}

		UUID transformUUID = object["Components"][0]["UUID"].as<uint64_t>();
		if (flags & Serializer::Flags::GenerateNewUUIDs)
		{
			if (uuidRemapper.find(transformUUID) != uuidRemapper.end())
			{
				/* Use existing newly generated UUID */
				transformUUID = uuidRemapper.at(transformUUID);
			}
			else
			{
				/* Generate new UUID */
				UUID newUUID = UUID();
				uuidRemapper.emplace(transformUUID, newUUID);
				transformUUID = newUUID;
			}
		}

		EntitySceneObject* pObject = (EntitySceneObject*)pScene->CreateEmptyObject(name, uuid, transformUUID);
		node = object["Components"];

		if (parentUuid != NULL)
		{
			SceneObject* pParent = pScene->FindSceneObject(parentUuid);
			if (pParent == nullptr) pScene->DelayedSetParent(pObject, parentUuid);
			else pObject->SetParent(pParent);
		}

		size_t currentComponentIndex = 0;

		const uint32_t transformTypeHash = ResourceType::GetHash(typeid(Transform));

		for (size_t i = 0; i < node.size(); ++i)
		{
			YAML::Node nextObject = node[i];
			YAML::Node subNode;
			UUID compUUID;
			uint32_t typeHash = 0;
			std::string typeName = "";
			YAML_READ(nextObject, subNode, UUID, compUUID, uint64_t);
			YAML_READ(nextObject, subNode, TypeName, typeName, std::string);
			YAML_READ(nextObject, subNode, TypeHash, typeHash, uint32_t);

			if (flags & Serializer::Flags::GenerateNewUUIDs)
			{
				if (uuidRemapper.find(compUUID) != uuidRemapper.end())
				{
					/* Use existing newly generated UUID */
					compUUID = uuidRemapper.at(compUUID);
				}
				else
				{
					/* Generate new UUID */
					UUID newUUID = UUID();
					uuidRemapper.emplace(compUUID, newUUID);
					compUUID = newUUID;
				}
			}

			Entity entityHandle = pObject->GetEntityHandle();
			EntityID entity = entityHandle.GetEntityID();
			EntityRegistry* pRegistry = pScene->GetRegistry();

			void* pComponentAddress = nullptr;
			if (typeHash != transformTypeHash) pComponentAddress = pRegistry->CreateComponent(entity, typeHash, compUUID);
			else pComponentAddress = pRegistry->GetComponentAddress(entity, compUUID);

			const TypeData* pTypeData = Reflect::GetTyeData(typeHash);
			PropertySerializer::DeserializeProperty(pTypeData, pComponentAddress, nextObject["Properties"]);

			pRegistry->GetTypeView(typeHash)->Invoke(InvocationType::OnValidate, pRegistry, entity, pComponentAddress);
			++currentComponentIndex;
		}

		pObject->SetActive(active);
		return pObject;
	}
}
