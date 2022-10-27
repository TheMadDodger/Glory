#include <glm/glm.hpp>
#include <JobManager.h>
#include "EntityScene.h"
#include "Entity.h"
#include "Components.h"
#include "Systems.h"
#include "EntitySceneObject.h"




#include <Game.h>
#include <Engine.h>

namespace Glory
{
	EntityScene::EntityScene() : m_Valid(true), GScene("New Scene") //: m_pJobPool(nullptr)
	{
		APPEND_TYPE(EntityScene);
	}

	EntityScene::EntityScene(const std::string& sceneName) : m_Valid(true), GScene(sceneName)
	{
		APPEND_TYPE(EntityScene);
	}

	EntityScene::EntityScene(const std::string& sceneName, UUID uuid) : m_Valid(true), GScene(sceneName, uuid)
	{
		APPEND_TYPE(EntityScene);
	}

	EntityScene::~EntityScene()
	{
		m_Valid = false;
	}

	Entity EntityScene::CreateEntity(UUID uuid)
	{
		EntityID entityID = m_Registry.CreateEntity<Transform>(uuid);
		return Entity(entityID, this);
	}

	EntitySceneObject* EntityScene::GetEntitySceneObjectFromEntityID(EntityID entity)
	{
		if (m_EntityIDToObject.find(entity) == m_EntityIDToObject.end()) return nullptr;
		return m_EntityIDToObject[entity];
	}

	EntityRegistry* EntityScene::GetRegistry()
	{
		return &m_Registry;
	}

	GLORY_API bool EntityScene::IsValid() const
	{
		return m_Valid;
	}

	void EntityScene::Initialize()
	{
		// Register engine systems
		m_Registry.RegisterComponent<Transform>();
		m_Registry.RegisterComponent<MeshRenderer>();
		m_Registry.RegisterComponent<MeshFilter>();
		m_Registry.RegisterComponent<CameraComponent>();
		m_Registry.RegisterComponent<LookAt>();
		m_Registry.RegisterComponent<Spin>();
		m_Registry.RegisterComponent<LightComponent>();
	}

	void EntityScene::OnTick()
	{
		//m_Registry.Update();
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}

	void EntityScene::OnPaint()
	{
		//m_Registry.Draw();
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}

	SceneObject* EntityScene::CreateObject(const std::string& name)
	{
		UUID uuid = UUID();
		Entity entity = CreateEntity(uuid);
		return new EntitySceneObject(entity, name, uuid);
	}

	SceneObject* EntityScene::CreateObject(const std::string& name, UUID uuid)
	{
		Entity entity = CreateEntity(uuid);
		return new EntitySceneObject(entity, name, uuid);
	}

	void EntityScene::OnDeleteObject(SceneObject* pObject)
	{
		
	}

	void EntityScene::OnObjectAdded(SceneObject* pObject)
	{
		EntitySceneObject* pEntityObject = (EntitySceneObject*)pObject;
		EntityID entity = pEntityObject->GetEntityHandle().GetEntityID();
		m_EntityIDToObject[entity] = pEntityObject;
	}
}