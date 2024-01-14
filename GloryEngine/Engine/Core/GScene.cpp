#include "GScene.h"
#include "Entity.h"
#include "Components.h"
#include "PrefabData.h"
#include "UUIDRemapper.h"
#include "DistributedRandom.h"
#include "PropertySerializer.h"
#include "SceneManager.h"
#include "Engine.h"

#include <NodeRef.h>

namespace Glory
{
	GScene::GScene(const std::string& sceneName) : Resource(sceneName), m_Registry(this)
	{
		APPEND_TYPE(GScene);
	}
		
	GScene::GScene(const std::string& sceneName, UUID uuid) : Resource(uuid, sceneName), m_Registry(this)
	{
		APPEND_TYPE(GScene);
	}
		
	GScene::~GScene()
	{
	}
		
	Entity GScene::CreateEmptyObject(UUID uuid, UUID transUuid)
	{
		return CreateEmptyObject("Empty Object", uuid, transUuid);
	}
		
	Entity GScene::CreateEmptyObject(const std::string& name, UUID uuid, UUID transUuid)
	{
		Entity entity = CreateEntity(transUuid);
		m_Ids.emplace(uuid, entity.GetEntityID());
		m_UUIds.emplace(entity.GetEntityID(), uuid);
		m_Names.emplace(entity.GetEntityID(), name);
		return entity;
	}
		
	size_t GScene::SceneObjectsCount() const
	{
		return m_Registry.Alive();
	}

	Entity GScene::GetEntityByUUID(UUID uuid)
	{
		const auto itor = m_Ids.find(uuid);
		if (itor == m_Ids.end())
			return {};

		const Utils::ECS::EntityID entity = itor->second;
		return { entity, this };
	}

	Entity GScene::GetEntityByEntityID(Utils::ECS::EntityID entityId)
	{
		return { entityId, this };
	}

	void GScene::DestroyEntity(Utils::ECS::EntityID entity)
	{
		const auto itor = m_UUIds.find(entity);
		if (itor == m_UUIds.end()) return;
		m_Registry.DestroyEntity(entity);
		const UUID uuid = itor->second;
		m_UUIds.erase(itor);
		m_Ids.erase(uuid);
	}
		
	void GScene::Start()
	{
		m_Registry.InvokeAll(Utils::ECS::InvocationType::Start);
	}
		
	void GScene::Stop()
	{
		m_Registry.InvokeAll(Utils::ECS::InvocationType::Stop);
	}

	void GScene::SetPrefab(Utils::ECS::EntityID entity, UUID prefabID)
	{
		const auto itor = m_UUIds.find(entity);
		if (itor == m_UUIds.end()) return;
		const UUID uuid = itor->second;

		m_ActivePrefabs.emplace(uuid, prefabID);

		Utils::ECS::EntityView* pEntityView = m_Registry.GetEntityView(entity);
		for (size_t i = 0; i < pEntityView->ChildCount(); ++i)
		{
			SetChildrenPrefab(pEntityView->Child(i), prefabID);
		}
	}

	void GScene::UnsetPrefab(Utils::ECS::EntityID entity)
	{
		const auto itor = m_UUIds.find(entity);
		if (itor == m_UUIds.end()) return;
		const UUID uuid = itor->second;

		m_ActivePrefabs.erase(uuid);

		Utils::ECS::EntityView* pEntityView = m_Registry.GetEntityView(entity);
		for (size_t i = 0; i < pEntityView->ChildCount(); ++i)
		{
			UnsetChildrenPrefab(pEntityView->Child(i));
		}
	}

	const UUID GScene::Prefab(UUID objectID) const
	{
		const auto itor = m_ActivePrefabs.find(objectID);
		return itor != m_ActivePrefabs.end() ? itor->second : 0;
	}

	const UUID GScene::PrefabChild(UUID objectID) const
	{
		const auto itor = m_ActivePrefabChildren.find(objectID);
		return itor != m_ActivePrefabChildren.end() ? itor->second : 0;
	}

	Utils::ECS::EntityRegistry& GScene::GetRegistry()
	{
		return m_Registry;
	}

	UUID GScene::GetEntityUUID(Utils::ECS::EntityID entity) const
	{
		return m_UUIds.at(entity);
	}

	Utils::ECS::EntityID GScene::Parent(Utils::ECS::EntityID entity) const
	{
		return m_Registry.GetParent(entity);
	}

	Utils::ECS::EntityID GScene::Parent(UUID uuid) const
	{
		const auto itor = m_Ids.find(uuid);
		if (itor == m_Ids.end())
			return {};

		return Parent(itor->second);
	}

	void GScene::SetParent(Utils::ECS::EntityID entity, Utils::ECS::EntityID parent)
	{
		if (!m_Registry.SetParent(entity, parent)) return;
		//m_Registry.GetComponent<Transform>(entity).Parent = { entity, this };
	}

	size_t GScene::ChildCount(Utils::ECS::EntityID entity) const
	{
		return m_Registry.ChildCount(entity);
	}

	Utils::ECS::EntityID GScene::Child(Utils::ECS::EntityID entity, size_t index) const
	{
		return m_Registry.Child(entity, index);
	}

	Entity GScene::ChildEntity(Utils::ECS::EntityID entity, size_t index)
	{
		return GetEntityByEntityID(Child(entity, index));
	}

	size_t GScene::SiblingIndex(Utils::ECS::EntityID entity) const
	{
		return m_Registry.SiblingIndex(entity);
	}

	void GScene::SetSiblingIndex(Utils::ECS::EntityID entity, size_t index)
	{
		return m_Registry.SetSiblingIndex(entity, index);
	}

	std::string_view GScene::EntityName(Utils::ECS::EntityID entity) const
	{
		const auto itor = m_Names.find(entity);
		if (itor == m_Names.end()) return "";
		return itor->second;
	}

	void GScene::SetEntityName(Utils::ECS::EntityID entity, const std::string_view name)
	{
		const auto itor = m_Names.find(entity);
		if (itor == m_Names.end()) return;
		itor->second = name;
	}

	Entity GScene::InstantiatePrefab(UUID parent, PrefabData* pPrefab, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
	{
		const UUID uuid = UUID();
		const uint32_t first32Bits = uint32_t((uuid << 32) >> 32);
		const uint32_t second32Bits = uint32_t(uuid >> 32);
		const uint32_t seed = first32Bits & second32Bits;
		UUIDRemapper remapper{ seed };
		remapper.EnforceRemap(pPrefab->RootNode().OriginalUUID(), uuid);
		return InstantiatePrefab(parent, pPrefab, RandomDevice::Seed(), pos, rot, scale);
	}

	Entity GScene::InstantiatePrefab(UUID parent, PrefabData* pPrefab, uint32_t remapSeed, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
	{
		UUIDRemapper remapper{ remapSeed };
		const PrefabNode& rootNode = pPrefab->RootNode();
		Entity entity = InstantiatePrefabNode(parent, rootNode, remapper);

		Transform& transform = entity.GetComponent<Transform>();
		transform.Position = pos;
		transform.Rotation = rot;
		transform.Scale = scale;

		SetPrefab(entity.GetEntityID(), pPrefab->GetUUID());
		return entity;
	}

	Entity GScene::InstantiatePrefab(UUID parent, PrefabData* pPrefab, UUIDRemapper& remapper, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
	{
		const PrefabNode& rootNode = pPrefab->RootNode();
		Entity entity = InstantiatePrefabNode(parent, rootNode, remapper);

		Transform& transform = entity.GetComponent<Transform>();
		transform.Position = pos;
		transform.Rotation = rot;
		transform.Scale = scale;

		SetPrefab(entity.GetEntityID(), pPrefab->GetUUID());
		return entity;
	}

	void GScene::DelayedSetParent(Entity entity, UUID parentID)
	{
		if (!entity.IsValid() || parentID == NULL) return;
		m_DelayedParents.emplace_back(DelayedParentData(entity, parentID));
	}

	void GScene::HandleDelayedParents()
	{
		std::for_each(m_DelayedParents.begin(), m_DelayedParents.end(), [&](DelayedParentData& data) {
			Entity parent = GetEntityByUUID(data.ParentID);
			if (!parent.IsValid())
			{
				std::string name{data.ObjectToParent.Name()};
				//m_pEngine->GetDebug().LogError("Could not set delayed parent for object " + name + " because the parent does not exist!");
				return;
			}
			data.ObjectToParent.SetParent(parent.GetEntityID());
		});
		m_DelayedParents.clear();
	}

	void GScene::OnTick()
	{
		m_Registry.InvokeAll(Glory::Utils::ECS::InvocationType::Update);
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}

	void GScene::OnPaint()
	{
		m_Registry.InvokeAll(Glory::Utils::ECS::InvocationType::Draw);
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}

	void GScene::SetUUID(UUID uuid)
	{
		m_ID = uuid;
	}

	Entity GScene::CreateEntity(UUID transUUID)
	{
		Utils::ECS::EntityID entityID = m_Registry.CreateEntity<Transform>(transUUID);
		return { entityID, this };
	}

	void GScene::SetChildrenPrefab(Utils::ECS::EntityID entity, UUID prefabID)
	{
		const auto itor = m_UUIds.find(entity);
		if (itor == m_UUIds.end()) return;
		const UUID uuid = itor->second;
		m_ActivePrefabChildren.emplace(uuid, prefabID);

		Utils::ECS::EntityView* pEntityView = m_Registry.GetEntityView(entity);
		for (size_t i = 0; i < pEntityView->ChildCount(); ++i)
		{
			SetChildrenPrefab(pEntityView->Child(i), prefabID);
		}
	}
	
	void GScene::UnsetChildrenPrefab(Utils::ECS::EntityID entity)
	{
		const auto itor = m_UUIds.find(entity);
		if (itor == m_UUIds.end()) return;
		const UUID uuid = itor->second;
		m_ActivePrefabChildren.erase(uuid);

		Utils::ECS::EntityView* pEntityView = m_Registry.GetEntityView(entity);
		for (size_t i = 0; i < pEntityView->ChildCount(); ++i)
		{
			UnsetChildrenPrefab(pEntityView->Child(i));
		}
	}

	Entity GScene::InstantiatePrefabNode(UUID parent, const PrefabNode& node, UUIDRemapper& remapper)
	{
		const UUID objectID = remapper(node.OriginalUUID());
		const UUID transformID = remapper(node.TransformUUID());
		Entity entity = CreateEmptyObject(node.Name(), objectID, transformID);
		const Utils::ECS::EntityID entityID = entity.GetEntityID();
		const Entity parentEntity = GetEntityByUUID(parent);

		if (parentEntity.IsValid())
			SetParent(entity.GetEntityID(), parentEntity.GetEntityID());
		
		entity.SetActive(node.ActiveSelf());
		
		const std::string& serializedComponents = node.SerializedComponents();
		YAML::Node components = YAML::Load(serializedComponents);
		
		const uint32_t transformTypeHash = ResourceTypes::GetHash(typeid(Transform));
		const uint32_t scriptedTypeHash = ResourceTypes::GetHash(typeid(ScriptedComponent));
		
		size_t currentComponentIndex = 0;
		for (size_t i = 0; i < components.size(); ++i)
		{
			YAML::Node nextObject = components[i];
			YAML::Node subNode;
			uint32_t typeHash = 0;
			UUID originalUUID = 0;
			std::string typeName = "";
			YAML_READ(nextObject, subNode, TypeHash, typeHash, uint32_t);
			YAML_READ(nextObject, subNode, UUID, originalUUID, uint64_t);

			UUID compUUID = remapper(originalUUID);

			void* pComponentAddress = nullptr;
			if (typeHash != transformTypeHash) pComponentAddress = m_Registry.CreateComponent(entityID, typeHash, compUUID);
			else
			{
				Utils::ECS::EntityView* pEntityView = m_Registry.GetEntityView(entityID);
				compUUID = pEntityView->ComponentUUIDAt(0);
				pComponentAddress = m_Registry.GetComponentAddress(entityID, compUUID);
			}

			const Utils::Reflect::TypeData* pTypeData = Utils::Reflect::Reflect::GetTyeData(typeHash);
			YAML::Node originalProperties = nextObject["Properties"];
			if (typeHash != scriptedTypeHash)
			{
				m_pManager->GetEngine()->GetSerializers().DeserializeProperty(pTypeData, pComponentAddress, originalProperties);
			}
			else
			{
				YAML::Node finalProperties = YAML::Node(YAML::NodeType::Map);
		
				Utils::NodeRef originalPropertiesRef = originalProperties;
				Utils::NodeRef finalPropertiesRef = finalProperties;
		
				Utils::NodeValueRef props = originalPropertiesRef.ValueRef();
				Utils::NodeValueRef finalProps = finalPropertiesRef.ValueRef();
		
				finalPropertiesRef["m_Script"].Set(originalPropertiesRef["m_Script"].As<uint64_t>());
				YAML::Node scriptData = originalProperties["ScriptData"];
				for (YAML::const_iterator itor = scriptData.begin(); itor != scriptData.end(); ++itor)
				{
					const std::string name = itor->first.as<std::string>();
					Utils::NodeValueRef prop = props["ScriptData"][name];
					if (!prop.IsMap())
					{
						finalProps[name].Set(prop.Node());
						continue;
					}
		
					Utils::NodeValueRef originalSceneUUD = prop["SceneUUID"];
					Utils::NodeValueRef originalObjectUUD = prop["ObjectUUID"];
		
					Utils::NodeValueRef sceneUUID = finalProps["ScriptData"][name]["SceneUUID"];
					Utils::NodeValueRef objectUUID = finalProps["ScriptData"][name]["ObjectUUID"];
		
					if (!originalSceneUUD.Exists() || !originalObjectUUD.Exists())
					{
						finalProps["ScriptData"][name].Set(prop.Node());
						continue;
					}
		
					sceneUUID.Set((uint64_t)GetUUID());
					const UUID uuid = originalObjectUUD.As<uint64_t>();
					UUID remapped;
					if (!remapper.Find(uuid, remapped))
						remapped = uuid;
		
					objectUUID.Set(remapped);
				}

				
				m_pManager->GetEngine()->GetSerializers().DeserializeProperty(pTypeData, pComponentAddress, finalProperties);
			}
		
			m_Registry.GetTypeView(typeHash)->Invoke(Utils::ECS::InvocationType::OnValidate, &m_Registry, entityID, pComponentAddress);
			++currentComponentIndex;
		}
		
		for (size_t i = 0; i < node.ChildCount(); ++i)
		{
			const PrefabNode& childNode = node.ChildNode(i);
			InstantiatePrefabNode(objectID, childNode, remapper);
		}
		
		return entity;
	}

	SceneManager* GScene::Manager()
	{
		return m_pManager;
	}

	void GScene::SetManager(SceneManager* pManager)
	{
		m_pManager = pManager;
	}
}
