#include "GScene.h"
#include "Entity.h"
#include "Components.h"
#include "PrefabData.h"
#include "UUIDRemapper.h"
#include "DistributedRandom.h"
#include "PropertySerializer.h"
#include "SceneManager.h"
#include "Engine.h"
#include "Debug.h"
#include "BinaryStream.h"
#include "BinarySerialization.h"
#include "ComponentHelpers.h"

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
		m_Registry.InvokeAll(Utils::ECS::InvocationType::OnValidate, NULL);
		m_Registry.InvokeAll(Utils::ECS::InvocationType::Start, NULL);
		m_Registry.InvokeAll(Utils::ECS::InvocationType::OnEnable,
		[](Utils::ECS::BaseTypeView* pTypeView, Utils::ECS::EntityView* pEntity, size_t componentIndex) {
			const bool isActive = pEntity->IsActive() && pTypeView->IsActiveByIndex(componentIndex);
			return isActive;
		});
	}
		
	void GScene::Stop()
	{
		m_Registry.InvokeAll(Utils::ECS::InvocationType::OnDisable,
		[](Utils::ECS::BaseTypeView* pTypeView, Utils::ECS::EntityView* pEntity, size_t componentIndex) {
			const bool isActive = pEntity->IsActive() && pTypeView->IsActiveByIndex(componentIndex);
			return isActive;
		});
		m_Registry.InvokeAll(Utils::ECS::InvocationType::Stop, NULL);
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

		const Utils::ECS::EntityID root = pPrefab->Child(0, 0);
		remapper.EnforceRemap(pPrefab->GetEntityUUID(root), uuid);
		return InstantiatePrefab(parent, pPrefab, remapper, pos, rot, scale);
	}

	Entity GScene::InstantiatePrefab(UUID parent, PrefabData* pPrefab, uint32_t remapSeed, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
	{
		auto itor = m_Ids.find(parent);
		const Utils::ECS::EntityID parentEntity = itor != m_Ids.end() ? itor->second : 0;

		UUIDRemapper remapper{ remapSeed };
		Entity entity = Instantiate(pPrefab, remapper, parentEntity);

		Transform& transform = entity.GetComponent<Transform>();
		transform.Position = pos;
		transform.Rotation = rot;
		transform.Scale = scale;

		entity.SetDirty();

		SetPrefab(entity.GetEntityID(), pPrefab->GetUUID());
		return entity;
	}

	Entity GScene::InstantiatePrefab(UUID parent, PrefabData* pPrefab, UUIDRemapper& remapper, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
	{
		auto itor = m_Ids.find(parent);
		const Utils::ECS::EntityID parentEntity = itor != m_Ids.end() ? itor->second : 0;
		Entity entity = Instantiate(pPrefab, remapper, parentEntity);

		Transform& transform = entity.GetComponent<Transform>();
		transform.Position = pos;
		transform.Rotation = rot;
		transform.Scale = scale;

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

	void GScene::OnTick()
	{
		if (m_MarkedForDestruct) return;
		m_Registry.InvokeAll(Glory::Utils::ECS::InvocationType::Update, NULL);
	}

	void GScene::OnPaint()
	{
		if (m_MarkedForDestruct) return;
		m_Registry.InvokeAll(Glory::Utils::ECS::InvocationType::Draw, NULL);
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

	Entity GScene::InstantiateEntity(GScene* pOther, UUIDRemapper& IDRemapper, Utils::ECS::EntityID entity,
		std::vector<Utils::ECS::EntityID>& newEntities, Utils::ECS::EntityID parent)
	{
		const Utils::ECS::EntityID newEntity = m_Registry.CreateEntity();
		newEntities.push_back(newEntity);
		Utils::ECS::EntityView* pNewEntityView = m_Registry.GetEntityView(newEntity);

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

		if (parent) m_Registry.SetParent(newEntity, parent);
		Utils::ECS::EntityView* pEntityView = pOther->m_Registry.GetEntityView(entity);
		pNewEntityView->Active() = pEntityView->Active();
		for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
		{
			const uint32_t type = pEntityView->ComponentTypeAt(i);
			const UUID uuid = pEntityView->ComponentUUIDAt(i);
			const UUID remappedID = IDRemapper(uuid);
			void* data = pOther->m_Registry.GetComponentAddress(entity, uuid);
			void* newComponent = m_Registry.CopyComponent(newEntity, type, remappedID, data);
			m_pManager->TriggerOnCopy(type, this, newComponent, remappedID, IDRemapper);
		}

		for (size_t i = 0; i < pOther->ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = pOther->Child(entity, i);
			InstantiateEntity(pOther, IDRemapper, child, newEntities, newEntity);
		}

		return Entity{ newEntity, this };
	}

	void GScene::MarkForDestruction()
	{
		m_MarkedForDestruct = true;
	}

	Entity GScene::Instantiate(GScene* pOther, UUIDRemapper& IDRemapper, Utils::ECS::EntityID parent)
	{
		Entity firstEntity;

		std::vector<Utils::ECS::EntityID> newEntities;
		for (size_t i = 0; i < pOther->ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = pOther->Child(0, i);
			const Entity nextEntity = InstantiateEntity(pOther, IDRemapper, child, newEntities, parent);
			if (i == 0)
				firstEntity = nextEntity;
		}

		m_Registry.InvokeAll(Utils::ECS::InvocationType::OnValidate, newEntities);
		m_Registry.InvokeAll(Utils::ECS::InvocationType::Start, newEntities);
		for (const Utils::ECS::EntityID entity: newEntities)
		{
			Entity entityHandle{ entity, this };
			Components::CallOnEnable(entityHandle);
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
	}

	void GScene::Serialize(BinaryStream& container) const
	{
		const size_t entityCount = m_Registry.Alive();
		const size_t typeViewCount = m_Registry.TypeViewCount();
		container.Write(entityCount).Write(typeViewCount);

		/* Serialize component datas */
		for (size_t i = 0; i < typeViewCount; ++i)
		{
			Utils::ECS::BaseTypeView* pTypeView = m_Registry.TypeViewAt(i);
			const uint32_t hash = pTypeView->ComponentTypeHash();
			container.Write(hash);
			container.Write(pTypeView->Size());

			const Utils::Reflect::TypeData* pTypeData =
				Utils::Reflect::Reflect::GetTyeData(hash);

			const int bufferOffset = pTypeData->DataBufferOffset();
			const size_t bufferSize = pTypeData->DataBufferSize();
			for (size_t j = 0; j < pTypeView->Size(); ++j)
			{
				Utils::ECS::EntityID entity = pTypeView->EntityAt(j);
				container.Write(entity);
				void* data = pTypeView->GetComponentAddress(entity);
				for (size_t k = 0; k < pTypeData->FieldCount(); ++k)
				{
					const Utils::Reflect::FieldData* pField = pTypeData->GetFieldData(k);
					void* pAddress = pField->GetAddress(data);
					SerializeData(container, pField, pAddress);
				}
				if (bufferOffset != -1)
				{
					const char* pBuffer = (const char*)data + bufferOffset;
					container.Write(pBuffer, bufferSize);
				}
			}
			container.Write(pTypeView->ActiveStates());
		}

		/* Serialize the hierarchy */
		SerializeTree(container, m_Registry, 0);
		for (auto itor = m_Registry.EntityViewBegin(); itor != m_Registry.EntityViewEnd(); ++itor)
		{
			const Utils::ECS::EntityID entity = itor->first;
			SerializeTree(container, m_Registry, entity);
		}

		/* Serialize scene IDs and names */
		container.Write(m_Ids.size());
		for (auto itor = m_Ids.begin(); itor != m_Ids.end(); ++itor)
		{
			container.Write(itor->first).Write(itor->second).Write(m_Names.at(itor->second));
		}

		/* Serialize Scene settings */
		container.Write(m_Settings);
	}

	void GScene::Deserialize(BinaryStream& container)
	{
		size_t typeViewCount, entityCount;
		container.Read(entityCount).Read(typeViewCount);

		for (size_t i = 0; i < entityCount; ++i)
			m_Registry.CreateEntity();

		/* Deserialize component datas */
		for (size_t i = 0; i < typeViewCount; ++i)
		{
			uint32_t hash;
			container.Read(hash);
			Utils::ECS::BaseTypeView* pTypeView = m_Registry.GetTypeView(hash);

			size_t size;
			container.Read(size);

			const Utils::Reflect::TypeData* pTypeData =
				Utils::Reflect::Reflect::GetTyeData(hash);

			const int bufferOffset = pTypeData->DataBufferOffset();
			const size_t bufferSize = pTypeData->DataBufferSize();

			for (size_t j = 0; j < size; ++j)
			{
				Utils::ECS::EntityID entity;
				container.Read(entity);
				void* data = pTypeView->Create(entity);
				for (size_t k = 0; k < pTypeData->FieldCount(); ++k)
				{
					const Utils::Reflect::FieldData* pField = pTypeData->GetFieldData(k);
					void* pAddress = pField->GetAddress(data);
					DeserializeData(container, pField, pAddress);
				}

				if (bufferOffset != -1)
				{
					char* pBuffer = (char*)data + bufferOffset;
					container.Read(pBuffer, bufferSize);
				}
			}
			container.Read(pTypeView->ActiveStates());
		}

		/* Deserialize the hierarchy */
		for (size_t i = 0; i < entityCount + 1; ++i)
		{
			DeserializeTree(container, m_Registry);
		}

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
