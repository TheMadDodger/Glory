#include "EditorSceneSerializer.h"

#include <Serializers.h>
#include <PropertySerializer.h>
#include <AssetManager.h>
#include <PrefabData.h>
#include <Components.h>
#include <Engine.h>
#include <GScene.h>
#include <Reflection.h>
#include <NodeRef.h>

namespace Glory::Editor
{
	void EditorSceneSerializer::SerializeScene(Engine* pEngine, GScene* pScene, Utils::NodeRef node)
	{
		auto root = node.ValueRef();
		root.Set(YAML::Node(YAML::NodeType::Map));
		auto entities = root["Entities"];
		entities.Set(YAML::Node(YAML::NodeType::Sequence));
		/*
		 * Entities should be serialized in order of their hierarchies
		 * so that the sibling indices are preserved naturally.
		 */
		for (size_t i = 0; i < pScene->ChildCount(0); ++i)
		{
			Utils::ECS::EntityID child = pScene->Child(0, i);
			SerializeEntityRecursive(pEngine, pScene, child, entities);
		}
	}

	GScene* EditorSceneSerializer::DeserializeScene(Engine* pEngine, Utils::NodeRef node, UUID uuid, const std::string& name, Flags flags)
	{
		GScene* pScene = new GScene(name, uuid);
		DeserializeScene(pEngine, pScene, node, uuid, name, flags);
		return pScene;
	}

	void EditorSceneSerializer::DeserializeScene(Engine* pEngine, GScene* pScene, Utils::NodeRef node, UUID uuid, const std::string& name, Flags flags)
	{
		pScene->SetManager(pEngine->GetSceneManager());
		Utils::NodeValueRef entities = node["Entities"];
		for (size_t i = 0; i < entities.Size(); ++i)
		{
			Utils::NodeValueRef entity = entities[i];
			DeserializeEntity(pEngine, pScene, entity.Node(), flags);
		}
		pScene->HandleDelayedParents();
	}

	void EditorSceneSerializer::SerializeEntity(Engine* pEngine, GScene* pScene, Utils::ECS::EntityID entity, Utils::NodeValueRef entityNode)
	{
		Entity entityHandle = pScene->GetEntityByEntityID(entity);
		Utils::ECS::EntityRegistry& pRegistry = entityHandle.GetScene()->GetRegistry();
		Utils::ECS::EntityView* pEntityView = entityHandle.GetEntityView();
		
		Entity parent = entityHandle.ParentEntity();
		
		if (pScene->PrefabChild(entityHandle.EntityUUID())) return;

		entityNode.Set(YAML::Node(YAML::NodeType::Map));
		entityNode["Name"].Set(std::string{ entityHandle.Name() });
		entityNode["UUID"].Set(uint64_t(entityHandle.EntityUUID()));
		entityNode["Active"].Set(entityHandle.IsActiveSelf());
		entityNode["ParentUUID"].Set(uint64_t(parent.IsValid() ? parent.EntityUUID() : 0));
		
		const UUID prefabID = pScene->Prefab(entityHandle.EntityUUID());
		if (prefabID)
		{
			/* Serialize the prefab ID instead */
			entityNode["PrefabID"].Set(uint64_t(prefabID));
		
			/* Serialize ID remapping */
			PrefabData* pPrefab = pEngine->GetAssetManager().GetAssetImmediate<PrefabData>(prefabID);
			const Utils::ECS::EntityID root = pPrefab->Child(0, 0);
		
			if (pPrefab->GetEntityUUID(root) != entityHandle.EntityUUID())
			{
				const uint64_t id = entityHandle.EntityUUID();
				const uint32_t first32Bits = uint32_t((id << 32) >> 32);
				const uint32_t second32Bits = uint32_t(id >> 32);
				const uint32_t seed = first32Bits & second32Bits;
				entityNode["IDRemapSeed"].Set(seed);
			}
		
			/* For now only serialize the transform.
			 * We do not put this in a component array
			 * so that in future when we add overrides it can be ignored
			 * in scenes where this still occurs.
			 */
			const TypeData* pTransformTypeData = Transform::GetTypeData();
			void* pTransformAddress = pRegistry.GetComponentAddress(entity, pEntityView->ComponentUUIDAt(0));
			auto transform = entityNode["Transform"];
			transform.Set(YAML::Node(YAML::NodeType::Map));
			pEngine->GetSerializers().SerializeProperty("Properties", pTransformTypeData, pTransformAddress, transform);
		
			/* TODO: Serialize overrides */
			return;
		}

		auto components = entityNode["Components"];
		components.Set(YAML::Node(YAML::NodeType::Sequence));
		for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
		{
			//components.PushBack(YAML::Node(YAML::NodeType::Map));
			SerializeComponent(pEngine, &pScene->GetRegistry(), pEntityView, entity, i, components[i]);
		}
	}

	void EditorSceneSerializer::SerializeEntityRecursive(Engine* pEngine, GScene* pScene, Utils::ECS::EntityID entity, Utils::NodeValueRef entities)
	{
		const size_t index = entities.Size();
		entities.PushBack(YAML::Node(YAML::NodeType::Map));

		/* Serialize entity first then its children */
		SerializeEntity(pEngine, pScene, entity, entities[index]);

		for (size_t i = 0; i < pScene->ChildCount(entity); ++i)
		{
			Utils::ECS::EntityID child = pScene->Child(entity, i);
			SerializeEntityRecursive(pEngine, pScene, child, entities);
		}
	}

	Entity EditorSceneSerializer::DeserializeEntity(Engine* pEngine, GScene* pScene, Utils::NodeRef node, Flags flags)
	{
		UUIDRemapper& uuidRemapper = pEngine->m_UUIDRemapper;

		const std::string name = node["Name"].As<std::string>();
		UUID uuid = node["UUID"].As<uint64_t>();
		Utils::NodeValueRef activeNode = node["Active"];
		const bool active = activeNode.Exists() ? activeNode.As<bool>() : true;
		UUID parentUuid = node["ParentUUID"].As<uint64_t>();
		Utils::NodeValueRef remapSeedNode = node["IDRemapSeed"];
		uint32_t seed = remapSeedNode.Exists() ? remapSeedNode.As<uint32_t>() : 0;
		
		if (flags & Flags::GenerateNewUUIDs)
		{
			uuid = uuidRemapper(uuid);
			parentUuid = uuidRemapper(parentUuid);
		}
		
		Utils::NodeRef nodeRef{ node };
		Utils::NodeValueRef prefabIDRef = nodeRef["PrefabID"];
		if (!(flags & Flags::IgnorePrefabs) && prefabIDRef.Exists())
		{
			const UUID prefabID = prefabIDRef.As<uint64_t>();
			PrefabData* pPrefab = pEngine->GetAssetManager().GetAssetImmediate<PrefabData>(prefabID);
			if (pPrefab)
			{
				Utils::NodeValueRef idsRemapValue = nodeRef["IDRemap"];
				/* TODO: When GenerateNewUUIDs flag is set generate a new map of UUID remappings */
				Entity instantiatedEntity = {};
				/* Deserialize the transform override */
				Transform transform;
				Utils::NodeValueRef transformRef = nodeRef["Transform/Properties"];
				const glm::vec3 position = transformRef["Position"].As<glm::vec3>();
				const glm::quat rotation = transformRef["Rotation"].As<glm::quat>();
				const glm::vec3 scale = transformRef["Scale"].As<glm::vec3>();

				const Utils::ECS::EntityID root = pPrefab->Child(0, 0);
				const UUID rootID = pPrefab->GetEntityUUID(root);
				if (flags & Flags::GenerateNewUUIDs)
				{
					const uint32_t first32Bits = uint32_t((uuid << 32) >> 32);
					const uint32_t second32Bits = uint32_t(uuid >> 32);
					seed = first32Bits & second32Bits;
					uuidRemapper.SoftReset(seed);
					uuidRemapper.EnforceRemap(rootID, uuid);
					instantiatedEntity = pScene->InstantiatePrefab(0, pPrefab, uuidRemapper, position, rotation, scale);
				}
				else
				{
					UUIDRemapper remapper{ seed };
					remapper.EnforceRemap(rootID, uuid);
					instantiatedEntity = pScene->InstantiatePrefab(0, pPrefab, remapper, position, rotation, scale);
				}
		
				/* TODO: Deserialize overrides */
				return instantiatedEntity;
			}
		}
		
		UUID transformUUID = nodeRef["Components"][0]["UUID"].As<uint64_t>();
		if (flags & Flags::GenerateNewUUIDs)
		{
			transformUUID = uuidRemapper(transformUUID);
		}
		
		Entity entityHandle = pScene->CreateEmptyObject(name, uuid, transformUUID);
		Utils::ECS::EntityID entity = entityHandle.GetEntityID();

		if (parentUuid != NULL)
		{
			Entity parentEntity = pScene->GetEntityByUUID(parentUuid);
			if (!parentEntity.IsValid()) pScene->DelayedSetParent(entityHandle, parentUuid);
			else entityHandle.SetParent(parentEntity.GetEntityID());
		}
		
		size_t currentComponentIndex = 0;
		
		Utils::NodeValueRef components = node["Components"];
		for (size_t i = 0; i < components.Size(); ++i)
		{
			Utils::NodeValueRef component = components[i];
			DeserializeComponent(pEngine, pScene, entity, uuidRemapper, component.Node(), flags);
			++currentComponentIndex;
		}
		
		entityHandle.SetActive(active);
		return entityHandle;
	}

	void EditorSceneSerializer::SerializeComponent(Engine* pEngine, Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityView* pEntityView, Utils::ECS::EntityID entity, size_t index, Utils::NodeValueRef node)
	{
		node.Set(YAML::Node(YAML::NodeType::Map));
		const UUID compUUID = pEntityView->ComponentUUIDAt(index);
		node["UUID"].Set(uint64_t(compUUID));

		const uint32_t type = pEntityView->ComponentTypeAt(index);
		const Utils::Reflect::TypeData* pType = Utils::Reflect::Reflect::GetTyeData(type);

		node["TypeName"].Set(pType->TypeName());
		node["TypeHash"].Set(uint64_t(type));
		node["Active"].Set(pRegistry->GetTypeView(type)->IsActive(entity));

		pEngine->GetSerializers().SerializeProperty("Properties", pType, pRegistry->GetComponentAddress(entity, compUUID), node);
	}

	void EditorSceneSerializer::DeserializeComponent(Engine* pEngine, GScene* pScene, Utils::ECS::EntityID entity, UUIDRemapper& uuidRemapper, Utils::NodeRef node, Flags flags)
	{
		const bool noCallbacks = flags & Flags::NoComponentCallbacks;

		Utils::NodeRef component{ node };

		const uint32_t transformTypeHash = ResourceTypes::GetHash(typeid(Transform));

		UUID compUUID = component["UUID"].As<uint64_t>();
		Utils::NodeValueRef activeNode = component["Active"];
		const bool active = activeNode.Exists() ? activeNode.As<bool>() : true;
		const std::string typeName = component["TypeName"].As<std::string>();
		uint32_t typeHash = component["TypeHash"].As<uint32_t>();

		/* @todo: A scene migration system would be nice here */
		if (typeHash == 201832386)
			typeHash = 1226719936;

		if (flags & Flags::GenerateNewUUIDs)
		{
			compUUID = uuidRemapper(compUUID);
		}

		Utils::ECS::EntityRegistry& pRegistry = pScene->GetRegistry();

		void* pComponentAddress = nullptr;
		if (typeHash != transformTypeHash) pComponentAddress = pRegistry.CreateComponent(entity, typeHash, compUUID, !noCallbacks);
		else pComponentAddress = pRegistry.GetComponentAddress(entity, compUUID);

		const TypeData* pTypeData = Reflect::GetTyeData(typeHash);
		pEngine->GetSerializers().DeserializeProperty(pTypeData, pComponentAddress, component["Properties"]);

		Utils::ECS::BaseTypeView* pTypeView = pRegistry.GetTypeView(typeHash);
		pTypeView->SetActive(entity, active);

		if (noCallbacks) return;
		pTypeView->Invoke(Utils::ECS::InvocationType::OnValidate, &pRegistry, entity, pComponentAddress);
	}
}
