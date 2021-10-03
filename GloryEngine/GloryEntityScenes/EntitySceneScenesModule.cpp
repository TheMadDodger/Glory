#include "EntitySceneScenesModule.h"
#include "Entity.h"
#include "Systems.h"

namespace Glory
{
	EntitySceneScenesModule::EntitySceneScenesModule()
	{
	}

	EntitySceneScenesModule::~EntitySceneScenesModule()
	{
	}

	void EntitySceneScenesModule::Initialize()
	{
		Entity& entity = m_Scene.CreateEntity();
		entity.AddComponent<Triangle>();
		m_Scene.m_Registry.RegisterSystem<TransformSystem>();
		m_Scene.m_Registry.RegisterSystem<TriangleSystem>();
	}

	void EntitySceneScenesModule::Cleanup()
	{
	}

	void EntitySceneScenesModule::Tick()
	{
		m_Scene.m_Registry.Update();
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}

	void EntitySceneScenesModule::OnPaint()
	{
		m_Scene.m_Registry.Draw();
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}
}
