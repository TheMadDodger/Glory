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

	void EntitySceneObjectSerializer::SerializeComponent(GloryECS::EntityID entityID, GloryECS::EntityRegistry* pRegistry, UUID componentUUID, void* pAddress, const GloryReflect::TypeData* pTypeData, YAML::Emitter& out)
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

		const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
		PropertySerializer::DeserializeProperty(pTypeData, pComponentAddress, nextObject["Properties"]);

		pRegistry->GetTypeView(typeHash)->Invoke(InvocationType::OnValidate, pRegistry, entity, pComponentAddress);
	}

	void EntitySceneObjectSerializer::Serialize(EntitySceneObject* pObject, YAML::Emitter& out)
	{
		SceneObject* pParent = pObject->GetParent();

		out << YAML::Key << "Name";
		out << YAML::Value << pObject->Name();
		out << YAML::Key << "UUID";
		out << YAML::Value << pObject->GetUUID();
		out << YAML::Key << "Active";
		out << YAML::Value << pObject->IsActiveSelf();
		out << YAML::Key << "ParentUUID";
		out << YAML::Value << (pParent ? pParent->GetUUID() : 0);
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
			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
			SerializeComponent(entity.GetEntityID(), pRegistry, componentUUID, pAddress, pTypeData, out);
		}
		out << YAML::EndSeq;
	}

	Object* EntitySceneObjectSerializer::Deserialize(Object* pParent, YAML::Node& object, const std::string&, Flags flags)
	{
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

		EntityScene* pScene = (EntityScene*)pParent;

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

		uint32_t transformTypeHash = ResourceType::GetHash(typeid(Transform));

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

			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(typeHash);
			PropertySerializer::DeserializeProperty(pTypeData, pComponentAddress, nextObject["Properties"]);

			pRegistry->GetTypeView(typeHash)->Invoke(InvocationType::OnValidate, pRegistry, entity, pComponentAddress);
			++currentComponentIndex;
		}

		pObject->SetActive(active);
		return pObject;
	}
}
