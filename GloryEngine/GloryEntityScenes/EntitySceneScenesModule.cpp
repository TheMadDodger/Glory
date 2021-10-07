#include "EntitySceneScenesModule.h"
#include "Entity.h"
#include "Systems.h"
#include <VertexHelpers.h>

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
		// teeeeeeeeeest
		std::vector<float> vertices = {
			0.0f, -0.5f, 1.0f, 0.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
			-0.5f, 0.5f, 0.0f, 0.0f, 1.0f
		};

		std::vector<uint32_t> indices = {
			0, 1, 2
		};
		MeshData* pMeshData = new MeshData(3, sizeof(Vertex), vertices, 3, indices, { AttributeType::Float2, AttributeType::Float3 });
		ModelData* pModelData = new ModelData();
		pModelData->AddMesh(pMeshData);

		Entity& entity = m_Scene.CreateEntity();
		entity.AddComponent<Triangle>(pModelData);
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
