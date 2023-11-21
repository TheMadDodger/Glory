#include "GScene.h"
#include "Entity.h"
#include "Components.h"

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
		m_Ids.emplace(entity.GetEntityID(), uuid);
		m_Names.emplace(entity.GetEntityID(), name);
		return entity;
	}
		
	size_t GScene::SceneObjectsCount() const
	{
		return m_Registry.Alive();
	}

	Entity GScene::GetEntity(UUID uuid)
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

		/*for (size_t i = 0; i < pObject->m_pChildren.size(); ++i)
		{
			SceneObject* pChild = pObject->m_pChildren[i];
			SetChildrenPrefab(pChild, prefabID);
		}*/
	}

	void GScene::UnsetPrefab(Utils::ECS::EntityID entity)
	{
		const auto itor = m_UUIds.find(entity);
		if (itor == m_UUIds.end()) return;
		const UUID uuid = itor->second;

		m_ActivePrefabs.erase(uuid);

		/*for (size_t i = 0; i < pObject->m_pChildren.size(); ++i)
		{
			SceneObject* pChild = pObject->m_pChildren[i];
			UnsetChildrenPrefab(pChild);
		}*/
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

	void GScene::SetParent(Utils::ECS::EntityID entity, Utils::ECS::EntityID parent)
	{
		if (!m_Registry.SetParent(entity, parent)) return;
		//m_Registry.GetComponent<Transform>(entity).Parent = { entity, this };
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
	
		/*for (size_t i = 0; i < pObject->m_pChildren.size(); ++i)
		{
			SceneObject* pChild = pObject->m_pChildren[i];
			SetChildrenPrefab(pChild, prefabID);
		}*/
	}
	
	void GScene::UnsetChildrenPrefab(Utils::ECS::EntityID entity)
	{
		const auto itor = m_UUIds.find(entity);
		if (itor == m_UUIds.end()) return;
		const UUID uuid = itor->second;
		m_ActivePrefabChildren.erase(uuid);
	
		/*for (size_t i = 0; i < pObject->m_pChildren.size(); ++i)
		{
			SceneObject* pChild = pObject->m_pChildren[i];
			UnsetChildrenPrefab(pChild);
		}*/
	}
}
