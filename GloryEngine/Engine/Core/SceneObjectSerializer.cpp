#include "SceneObjectSerializer.h"
#include "EntityComponentObject.h"
#include "SceneManager.h"
#include "Components.h"
#include "PrefabData.h"
#include "AssetManager.h"

#include "PropertySerializer.h"
#include "DistributedRandom.h"

#include <NodeRef.h>

namespace Glory
{
	DistributedRandom<uint32_t> SeedRandomizer;

	SceneObjectSerializer::SceneObjectSerializer()
	{
	}

	SceneObjectSerializer::~SceneObjectSerializer()
	{
	}

	void SceneObjectSerializer::SerializeComponent(Glory::Utils::ECS::EntityID entityID, Glory::Utils::ECS::EntityRegistry* pRegistry, UUID componentUUID, void* pAddress, const TypeData* pTypeData, YAML::Emitter& out)
	{
		Utils::ECS::BaseTypeView* pTypeView = pRegistry->GetTypeView(pTypeData->TypeHash());

		out << YAML::BeginMap;
		out << YAML::Key << "UUID";
		out << YAML::Value << componentUUID;
		out << YAML::Key << "Active";
		out << YAML::Value << pTypeView->IsActive(entityID);
		out << YAML::Key << "TypeName";
		out << YAML::Value << pTypeData->TypeName();
		out << YAML::Key << "TypeHash";
		out << YAML::Value << pTypeData->TypeHash();
		PropertySerializer::SerializeProperty("Properties", pTypeData, pAddress, out);
		out << YAML::EndMap;
	}

	void SceneObjectSerializer::DeserializeComponent(GScene* pScene, SceneObject* pObject, size_t componentIndex, YAML::Node& object)
	{
		YAML::Node nextObject = object;
		YAML::Node subNode;
		UUID compUUID;
		bool active = true;
		uint32_t typeHash = 0;
		std::string typeName = "";
		YAML_READ(nextObject, subNode, UUID, compUUID, uint64_t);
		YAML_READ(nextObject, subNode, Active, active, bool);
		YAML_READ(nextObject, subNode, TypeName, typeName, std::string);
		YAML_READ(nextObject, subNode, TypeHash, typeHash, uint32_t);

		Entity entityHandle = pObject->GetEntityHandle();
		Utils::ECS::EntityID entity = entityHandle.GetEntityID();
		Utils::ECS::EntityRegistry* pRegistry = pScene->GetRegistry();

		void* pComponentAddress = pRegistry->CreateComponent(entity, typeHash, compUUID);
		Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entity);
		pEntityView->SetComponentIndex(pEntityView->ComponentCount() - 1, componentIndex);

		const TypeData* pTypeData = Reflect::GetTyeData(typeHash);
		PropertySerializer::DeserializeProperty(pTypeData, pComponentAddress, nextObject["Properties"]);

		Utils::ECS::BaseTypeView* pTypeView = pRegistry->GetTypeView(typeHash);
		pTypeView->SetActive(entity, active);

		pTypeView->Invoke(Utils::ECS::InvocationType::OnValidate, pRegistry, entity, pComponentAddress);
	}

	void SceneObjectSerializer::Serialize(SceneObject* pObject, YAML::Emitter& out)
	{
		Entity entity = pObject->GetEntityHandle();
		Utils::ECS::EntityRegistry* pRegistry = entity.GetScene()->GetRegistry();
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();

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

			/* Serialize ID remapping */
			PrefabData* pPrefab = AssetManager::GetAssetImmediate<PrefabData>(prefabID);
			const PrefabNode& rootNode = pPrefab->RootNode();

			if (rootNode.OriginalUUID() != pObject->GetUUID())
			{
				out << YAML::Key << "IDRemapSeed";
				const uint64_t id = pObject->GetUUID();
				const uint32_t first32Bits = uint32_t((id << 32) >> 32);
				const uint32_t second32Bits = uint32_t(id >> 32);
				const uint32_t seed = first32Bits & second32Bits;
				out << YAML::Value << seed;
			}

			/* For now only serialize the transform.
			 * We do not put this in a component array
			 * so that in future when we add overrides it can be ignored
			 * in scenes where this still occurs.
			 */
			const TypeData* pTransformTypeData = Transform::GetTypeData();
			void* pTransformAddress = pRegistry->GetComponentAddress(entity.GetEntityID(), pEntityView->ComponentUUIDAt(0));
			out << YAML::Key << "Transform";
			out << YAML::Value << YAML::BeginMap;
			PropertySerializer::SerializeProperty("Properties", pTransformTypeData, pTransformAddress, out);
			out << YAML::EndMap;

			/* TODO: Serialize overrides */
			return;
		}

		out << YAML::Key << "Components";
		out << YAML::Value << YAML::BeginSeq;
		for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
		{
			UUID componentUUID = pEntityView->ComponentUUIDAt(i);
			uint32_t typeHash = pEntityView->ComponentTypeAt(i);
			Utils::ECS::BaseTypeView* pTypeView = pRegistry->GetTypeView(typeHash);
			void* pAddress = pTypeView->GetComponentAddress(entity.GetEntityID());
			const TypeData* pTypeData = Reflect::GetTyeData(typeHash);
			SerializeComponent(entity.GetEntityID(), pRegistry, componentUUID, pAddress, pTypeData, out);
		}
		out << YAML::EndSeq;
	}

	Object* SceneObjectSerializer::Deserialize(Object* pParent, YAML::Node& object, const std::string&, Flags flags)
	{
		GScene* pScene = (GScene*)pParent;

		UUIDRemapper& uuidRemapper = GloryContext::GetContext()->m_UUIDRemapper;

		YAML::Node node;
		std::string name;
		UUID uuid;
		UUID parentUuid;
		uint32_t seed = 0;
		bool active = true;
		YAML_READ(object, node, Name, name, std::string);
		YAML_READ(object, node, UUID, uuid, uint64_t);
		YAML_READ(object, node, Active, active, bool);
		YAML_READ(object, node, ParentUUID, parentUuid, uint64_t);
		YAML_READ(object, node, IDRemapSeed, seed, uint32_t);

		if (flags & Serializer::Flags::GenerateNewUUIDs)
		{
			uuid = uuidRemapper(uuid);
			parentUuid = uuidRemapper(parentUuid);
		}

		Utils::NodeRef nodeRef{ object };
		Utils::NodeValueRef prefabIDRef = nodeRef["PrefabID"];
		if (!(flags & Flags::IgnorePrefabs) && prefabIDRef.Exists())
		{
			const UUID prefabID = prefabIDRef.As<uint64_t>();
			PrefabData* pPrefab = AssetManager::GetAssetImmediate<PrefabData>(prefabID);
			if (pPrefab)
			{
				Utils::NodeValueRef idsRemapValue = nodeRef["IDRemap"];
				/* TODO: When GenerateNewUUIDs flag is set generate a new map of UUID remappings */
				SceneObject* pInstantiatedPrefab = nullptr;
				/* Deserialize the transform override */
				Transform transform;
				Utils::NodeValueRef transformRef = nodeRef["Transform/Properties"];
				const glm::vec3 position = transformRef["Position"].As<glm::vec3>();
				const glm::quat rotation = transformRef["Rotation"].As<glm::quat>();
				const glm::vec3 scale = transformRef["Scale"].As<glm::vec3>();

				if (flags & Serializer::Flags::GenerateNewUUIDs)
				{
					const uint32_t first32Bits = uint32_t((uuid << 32) >> 32);
					const uint32_t second32Bits = uint32_t(uuid >> 32);
					seed = first32Bits & second32Bits;
					uuidRemapper.SoftReset(seed);
					uuidRemapper.EnforceRemap(pPrefab->RootNode().OriginalUUID(), uuid);
					pInstantiatedPrefab = pScene->InstantiatePrefab(nullptr, pPrefab, uuidRemapper, position, rotation, scale);
				}
				else
				{
					UUIDRemapper remapper{ seed };
					remapper.EnforceRemap(pPrefab->RootNode().OriginalUUID(), uuid);
					pInstantiatedPrefab = pScene->InstantiatePrefab(nullptr, pPrefab, remapper, position, rotation, scale);
				}

				/* TODO: Deserialize overrides */
				return pInstantiatedPrefab;
			}
		}

		UUID transformUUID = object["Components"][0]["UUID"].as<uint64_t>();
		if (flags & Serializer::Flags::GenerateNewUUIDs)
		{
			transformUUID = uuidRemapper(transformUUID);
		}

		SceneObject* pObject = pScene->CreateEmptyObject(name, uuid, transformUUID);
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
			bool active = true;
			uint32_t typeHash = 0;
			std::string typeName = "";
			YAML_READ(nextObject, subNode, UUID, compUUID, uint64_t);
			YAML_READ(nextObject, subNode, Active, active, bool);
			YAML_READ(nextObject, subNode, TypeName, typeName, std::string);
			YAML_READ(nextObject, subNode, TypeHash, typeHash, uint32_t);

			if (flags & Serializer::Flags::GenerateNewUUIDs)
			{
				compUUID = uuidRemapper(compUUID);
			}

			Entity entityHandle = pObject->GetEntityHandle();
			Utils::ECS::EntityID entity = entityHandle.GetEntityID();
			Utils::ECS::EntityRegistry* pRegistry = pScene->GetRegistry();

			void* pComponentAddress = nullptr;
			if (typeHash != transformTypeHash) pComponentAddress = pRegistry->CreateComponent(entity, typeHash, compUUID);
			else pComponentAddress = pRegistry->GetComponentAddress(entity, compUUID);

			const TypeData* pTypeData = Reflect::GetTyeData(typeHash);
			PropertySerializer::DeserializeProperty(pTypeData, pComponentAddress, nextObject["Properties"]);

			Utils::ECS::BaseTypeView* pTypeView = pRegistry->GetTypeView(typeHash);
			pTypeView->SetActive(entity, active);

			pTypeView->Invoke(Utils::ECS::InvocationType::OnValidate, pRegistry, entity, pComponentAddress);
			++currentComponentIndex;
		}

		pObject->SetActive(active);
		return pObject;
	}
}
