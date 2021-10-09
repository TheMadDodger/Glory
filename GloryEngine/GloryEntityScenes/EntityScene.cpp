#include "EntityScene.h"
#include <glm/glm.hpp>
#include <JobManager.h>
#include "Entity.h"
#include "Components.h"

namespace Glory
{
	EntityScene::EntityScene() //: m_pJobPool(nullptr)
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
}