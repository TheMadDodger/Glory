#pragma once
#include "LayerMask.h"
#include "UUID.h"

#include <glm/glm.hpp>

namespace Glory
{
	class MeshData;
	class MaterialData;

	struct ObjectData
	{
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
		uint64_t SceneID;
		uint64_t ObjectID;
	};

	struct RenderData
	{
	public:
		// Mesh
		MeshData* m_pMesh;
		UUID m_SceneID;
		UUID m_ObjectID;

		// World matrices
		glm::mat4 m_World;
		//size_t m_NumInstances;
		//
		// Material
		MaterialData* m_pMaterial;
		LayerMask m_LayerMask;
	};
}
