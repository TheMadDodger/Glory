#include "EntityScene.h"
#include <glm/glm.hpp>
#include "EntityGameObject.h"
#include <JobManager.h>
#include "Entity.h"

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
		m_Entities.push_back(Entity(entityID, this));
		return m_Entities[m_Entities.size() - 1];
	}

	void EntityScene::Update()
	{
		//if (m_pJobPool == nullptr) m_pJobPool = Jobs::JobManager::Run<void>();
		//m_pJobPool->StartQueue();
		//std::for_each(m_Entities.begin(), m_Entities.end(), [&](EntityGameObject& entity) { entity.Update(); });
		//m_pJobPool->EndQueue();
	}
}