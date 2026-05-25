#include "GScene.h"
#include "Entity.h"
#include "Components.h"
#include "PrefabData.h"
#include "UUIDRemapper.h"
#include "DistributedRandom.h"
#include "SceneManager.h"
#include "IEngine.h"
#include "Debug.h"
#include "Resources.h"

#include "TransformManager.h"

#include <BinaryStream.h>
#include <NodeRef.h>

namespace Glory
{
	GScene::GScene(const std::string& sceneName):
		Resource(sceneName), m_Registry(this)
	{
		APPEND_TYPE(GScene);
	}
		
	GScene::GScene(const std::string& sceneName, UUID uuid):
		Resource(uuid, sceneName), m_Registry(this)
	{
		APPEND_TYPE(GScene);
	}

	GScene::GScene(GScene&& other) noexcept: Resource(std::move(other)),
		m_Ids(std::move(other.m_Ids)),
		m_UUIds(std::move(other.m_UUIds)),
		m_Names(std::move(other.m_Names)),
		m_ActivePrefabs(std::move(other.m_ActivePrefabs)),
		m_ActivePrefabChildren(std::move(other.m_ActivePrefabChildren)),
		m_Registry(std::move(other.m_Registry)),
		m_DelayedParents(std::move(other.m_DelayedParents)),
		m_Settings(std::move(other.m_Settings)),
		m_pManager(other.m_pManager),
		m_MarkedForDestruct(false),
		m_Starting(false)
	{
		other.m_pManager = nullptr;
	}

	GScene& GScene::operator=(GScene&& other) noexcept
	{
		m_Inheritence = std::move(other.m_Inheritence);
		m_ID = other.m_ID;
		m_Name = std::move(other.m_Name);
		m_Ids = std::move(other.m_Ids);
		m_UUIds = std::move(other.m_UUIds);
		m_Names = std::move(other.m_Names);
		m_ActivePrefabs = std::move(other.m_ActivePrefabs);
		m_ActivePrefabChildren = std::move(other.m_ActivePrefabChildren);
		m_Registry = std::move(other.m_Registry);
		m_DelayedParents = std::move(other.m_DelayedParents);
		m_Settings = std::move(other.m_Settings);
		m_pManager = other.m_pManager;
		other.m_pManager = nullptr;
		m_MarkedForDestruct = false;
		m_Starting = false;
		return *this;
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
		return m_Registry.AliveCount();
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
		/* Destroy children first */
		Entity e = GetEntityByEntityID(entity);
		while (e.ChildCount())
		{
			DestroyEntity(e.Child(0));
		}

		const auto itor = m_UUIds.find(entity);
		if (itor == m_UUIds.end()) return;
		const UUID uuid = itor->second;

		if (m_pManager)
		{
			m_pManager->OnSceneObjectDestroyed(GetUUID(), uuid);
		}

		m_Registry.DestroyEntity(entity);
		m_UUIds.erase(itor);
		m_Ids.erase(uuid);
	}
		
	void GScene::Start()
	{
		m_Starting = true;
		m_Registry.Validate();
		m_Registry.Start();
		m_Registry.Activate();
		m_Registry.EnableDraw();
		m_Starting = false;
	}
		
	void GScene::Stop()
	{
		m_Registry.Deactivate();
		m_Registry.DisableDraw();
		m_Registry.Stop();
	}

	void GScene::SetPrefab(Utils::ECS::EntityID entity, UUID prefabID)
	{
		const auto itor = m_UUIds.find(entity);
		if (itor == m_UUIds.end()) return;
		const UUID uuid = itor->second;

		m_ActivePrefabs.emplace(uuid, prefabID);
		for (size_t i = 0; i < m_Registry.ChildCount(entity); ++i)
		{
			SetChildrenPrefab(m_Registry.Child(entity, i), prefabID);
		}
	}

	void GScene::UnsetPrefab(Utils::ECS::EntityID entity)
	{
		const auto itor = m_UUIds.find(entity);
		if (itor == m_UUIds.end()) return;
		const UUID uuid = itor->second;

		m_ActivePrefabs.erase(uuid);
		for (size_t i = 0; i < m_Registry.ChildCount(entity); ++i)
		{
			UnsetChildrenPrefab(m_Registry.Child(entity, i));
		}
	}

	const UUID GScene::Prefab(UUID objectID) const
	{
		const auto itor = m_ActivePrefabs.find(objectID);
		return itor != m_ActivePrefabs.end() ? itor->second.GetUUID() : UUID(0ull);
	}

	const UUID GScene::PrefabChild(UUID objectID) const
	{
		const auto itor = m_ActivePrefabChildren.find(objectID);
		return itor != m_ActivePrefabChildren.end() ? itor->second.GetUUID() : UUID(0ull);
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
		m_Registry.SetParent(entity, parent);
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

		const Utils::ECS::EntityID root = pPrefab->Child(0, 0);
		remapper.EnforceRemap(pPrefab->GetEntityUUID(root), uuid);
		return InstantiatePrefab(parent, pPrefab, remapper, pos, rot, scale);
	}

	Entity GScene::InstantiatePrefab(UUID parent, PrefabData* pPrefab, uint32_t remapSeed, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
	{
		auto itor = m_Ids.find(parent);
		const Utils::ECS::EntityID parentEntity = itor != m_Ids.end() ? itor->second : 0;

		UUIDRemapper remapper{ remapSeed };
		Entity entity = Instantiate(pPrefab, remapper, parentEntity, pos, rot, scale);

		entity.SetDirty();

		SetPrefab(entity.GetEntityID(), pPrefab->GetUUID());
		return entity;
	}

	Entity GScene::InstantiatePrefab(UUID parent, PrefabData* pPrefab, UUIDRemapper& remapper, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
	{
		auto itor = m_Ids.find(parent);
		const Utils::ECS::EntityID parentEntity = itor != m_Ids.end() ? itor->second : 0;
		Entity entity = Instantiate(pPrefab, remapper, parentEntity, pos, rot, scale);

		entity.SetDirty();

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

	bool GScene::IsStarting() const
	{
		return m_Starting;
	}

	void GScene::OnTick(float dt)
	{
		if (m_MarkedForDestruct) return;
		m_Registry.Update(dt);
	}

	void GScene::OnPaint()
	{
		if (m_MarkedForDestruct) return;
		m_Registry.Draw();
	}

	void GScene::SetUUID(UUID uuid)
	{
		m_ID = uuid;
	}

	Entity GScene::CreateEntity(UUID transUUID)
	{
		Utils::ECS::EntityID entityID = m_Registry.CreateEntity();
		m_Registry.AddComponent<Transform>(entityID, transUUID);
		return { entityID, this };
	}

	void GScene::SetChildrenPrefab(Utils::ECS::EntityID entity, UUID prefabID)
	{
		const auto itor = m_UUIds.find(entity);
		if (itor == m_UUIds.end()) return;
		const UUID uuid = itor->second;
		m_ActivePrefabChildren.emplace(uuid, prefabID);
		for (size_t i = 0; i < m_Registry.ChildCount(entity); ++i)
		{
			SetChildrenPrefab(m_Registry.Child(entity, i), prefabID);
		}
	}
	
	void GScene::UnsetChildrenPrefab(Utils::ECS::EntityID entity)
	{
		const auto itor = m_UUIds.find(entity);
		if (itor == m_UUIds.end()) return;
		const UUID uuid = itor->second;
		m_ActivePrefabChildren.erase(uuid);
		for (size_t i = 0; i < m_Registry.ChildCount(entity); ++i)
		{
			UnsetChildrenPrefab(m_Registry.Child(entity, i));
		}
	}

	Entity GScene::InstantiateEntity(GScene* pOther, UUIDRemapper& IDRemapper, Utils::ECS::EntityID entity,
		std::vector<Utils::ECS::EntityID>& newEntities, Utils::ECS::EntityID parent)
	{
		const Utils::ECS::EntityID newEntity = m_Registry.CreateEntity();
		newEntities.push_back(newEntity);

		const UUID entityID = pOther->GetEntityUUID(entity);
		const UUID remappedEntityID = IDRemapper(entityID);
		auto iter1 = m_Ids.find(remappedEntityID);
		auto iter2 = m_UUIds.find(newEntity);
		auto iter3 = m_Names.find(newEntity);

		if (iter1 != m_Ids.end())
		{
			m_pManager->GetEngine()->GetDebug().LogError("ID already exists!");
		}
		if (iter2 != m_UUIds.end())
		{
			m_pManager->GetEngine()->GetDebug().LogError("Entity ID already exists!");
		}
		if (iter3 != m_Names.end())
		{
			m_pManager->GetEngine()->GetDebug().LogError("Entity ID already has a name!");
		}

		m_Ids.emplace(remappedEntityID, newEntity);
		m_UUIds.emplace(newEntity, remappedEntityID);
		m_Names.emplace(newEntity, pOther->EntityName(entity));

		m_Registry.SetActive(newEntity, pOther->m_Registry.EntityActiveSelf(entity));
		if (parent) m_Registry.SetParent(newEntity, parent);
		for (size_t i = 0; i < pOther->m_Registry.EntityComponentCount(entity); ++i)
		{
			const uint32_t type = pOther->m_Registry.EntityComponentType(entity, i);
			const UUID uuid = pOther->m_Registry.EntityComponentID(entity, i);
			const UUID remappedID = IDRemapper(uuid);
			void* data = pOther->m_Registry.GetComponentAddress(entity, type);
			void* newComponent = m_Registry.CopyComponent(newEntity, type, remappedID, data);
			m_pManager->TriggerOnCopy(type, this, newComponent, remappedID, IDRemapper);
		}

		for (size_t i = 0; i < pOther->ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = pOther->Child(entity, i);
			InstantiateEntity(pOther, IDRemapper, child, newEntities, newEntity);
		}

		return { newEntity, this };
	}

	void GScene::References(IEngine* pEngine, std::vector<UUID>& references) const
	{
		if (m_Settings.m_LightingSettings.m_Skybox)
		{
			references.push_back(m_Settings.m_LightingSettings.m_Skybox);
			Resource* pResource = pEngine->GetResources().GetResource(m_Settings.m_LightingSettings.m_Skybox);
			if (pResource) pResource->References(pEngine, references);
		}
		if (m_Settings.m_LightingSettings.m_IrradianceMap)
		{
			references.push_back(m_Settings.m_LightingSettings.m_IrradianceMap);
			Resource* pResource = pEngine->GetResources().GetResource(m_Settings.m_LightingSettings.m_IrradianceMap);
			if (pResource) pResource->References(pEngine, references);
		}
		m_Registry.GetReferences(references);
	}

	void GScene::MarkForDestruction()
	{
		m_MarkedForDestruct = true;
	}

	Entity GScene::Instantiate(GScene* pOther, UUIDRemapper& IDRemapper, Utils::ECS::EntityID parent,
		const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
	{
		Entity firstEntity;

		std::vector<Utils::ECS::EntityID> newEntities;
		for (size_t i = 0; i < pOther->ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = pOther->Child(0, i);
			Entity nextEntity = InstantiateEntity(pOther, IDRemapper, child, newEntities, parent);
			if (i == 0)
				firstEntity = nextEntity;
		}

		if (firstEntity.IsValid())
		{
			Transform& transform = firstEntity.GetComponent<Transform>();
			transform.Position = pos;
			transform.Rotation = rot;
			transform.Scale = scale;
		}

		for (Utils::ECS::EntityID entity : newEntities)
		{
			m_Registry.CallOnValidate(entity);
			m_Registry.CallStart(entity);
		}

		for (Utils::ECS::EntityID entity : newEntities)
		{
			if (!m_Registry.EntityActiveHierarchy(entity)) continue;
			m_Registry.CallOnActivate(entity);
			m_Registry.CallOnEnableDraw(entity);
		}

		return firstEntity;
	}

	SceneManager* GScene::Manager()
	{
		return m_pManager;
	}

	void GScene::SetManager(SceneManager* pManager)
	{
		m_pManager = pManager;
		pManager->GetRegistryFactory().PopulateRegisry(m_Registry);
	}

	void GScene::Serialize(Utils::BinaryStream& container) const
	{
		m_Registry.Serialize(container);

		/* Serialize scene IDs and names */
		container.Write(m_Ids.size());
		for (auto itor = m_Ids.begin(); itor != m_Ids.end(); ++itor)
		{
			container.Write(itor->first).Write(itor->second).Write(m_Names.at(itor->second));
		}

		/* Serialize Scene settings */
		container.Write(m_Settings);
	}

	void GScene::Deserialize(Utils::BinaryStream& container)
	{
		m_Registry.Deserialize(container);

		/* Deserialize scene IDs and names */
		size_t idSize;
		container.Read(idSize);
		for (size_t i = 0; i < idSize; i++)
		{
			UUID id;
			Utils::ECS::EntityID entity;
			std::string name;
			container.Read(id).Read(entity).Read(name);
			m_Ids.emplace(id, entity);
			m_UUIds.emplace(entity, id);
			m_Names.emplace(entity, name);
		}

		/* Deserialize Scene settings */
		container.Read(m_Settings);
	}
}
