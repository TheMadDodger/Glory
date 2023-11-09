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
		return Entity{ entity, this };
	}

	Entity GScene::GetEntityByEntityID(Utils::ECS::EntityID entityId)
	{
		return Entity(entityId, this);
	}
		
	void GScene::Start()
	{
		m_Registry.InvokeAll(Utils::ECS::InvocationType::Start);
	}
		
	void GScene::Stop()
	{
		m_Registry.InvokeAll(Utils::ECS::InvocationType::Stop);
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
		return Entity(entityID, this);
	}
}
