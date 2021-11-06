#include <glm/glm.hpp>
#include <JobManager.h>
#include "EntityScene.h"
#include "Entity.h"
#include "Components.h"
#include "Systems.h"

namespace Glory
{
	EntityScene::EntityScene() : GScene("New Scene") //: m_pJobPool(nullptr)
	{
	}

	EntityScene::EntityScene(const std::string& sceneName) : GScene(sceneName)
	{
	}

	EntityScene::EntityScene(const std::string& sceneName, UUID uuid) : GScene(sceneName, uuid)
	{
	}

	EntityScene::~EntityScene()
	{
	}

	Entity& EntityScene::CreateEntity()
	{
		EntityID entityID = m_Registry.CreateEntity();
		m_Registry.AddComponent<Transform>(entityID, glm::vec3(), glm::vec3(), glm::vec3());
		m_Entities.push_back(Entity(entityID, this));
		return m_Entities[m_Entities.size() - 1];
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
}