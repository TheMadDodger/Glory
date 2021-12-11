#include <glm/glm.hpp>
#include <JobManager.h>
#include "EntityScene.h"
#include "Entity.h"
#include "Components.h"
#include "Systems.h"
#include "EntitySceneObject.h"

namespace Glory
{
	EntityScene::EntityScene() : GScene("New Scene") //: m_pJobPool(nullptr)
	{
		APPEND_TYPE(EntityScene);
	}

	EntityScene::EntityScene(const std::string& sceneName) : GScene(sceneName)
	{
		APPEND_TYPE(EntityScene);
	}

	EntityScene::EntityScene(const std::string& sceneName, UUID uuid) : GScene(sceneName, uuid)
	{
		APPEND_TYPE(EntityScene);
	}

	EntityScene::~EntityScene()
	{
	}

	Entity EntityScene::CreateEntity()
	{
		EntityID entityID = m_Registry.CreateEntity();
		m_Registry.AddComponent<Transform>(entityID);
		return Entity(entityID, this);
	}

	void EntityScene::Initialize()
	{
		m_Registry.RegisterSystem<TransformSystem>();
		m_Registry.RegisterSystem<MeshRenderSystem>();
	}

	void EntityScene::OnTick()
	{
		m_Registry.Update();
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}

	void EntityScene::OnPaint()
	{
		m_Registry.Draw();
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}

	SceneObject* EntityScene::CreateObject(const std::string& name)
	{
		Entity entity = CreateEntity();
		return new EntitySceneObject(entity, name);
	}

	SceneObject* EntityScene::CreateObject(const std::string& name, UUID uuid)
	{
		Entity entity = CreateEntity();
		return new EntitySceneObject(entity, name, uuid);
	}
}