#pragma once
#include <glm/glm.hpp>
#include "LayerMask.h"
#include "ModelData.h"
#include "MaterialData.h"

namespace Glory
{
	struct ObjectData
	{
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
		uint32_t ObjectID;
	};

	struct RenderData
	{
	public:
		// Mesh
		MeshData* m_pMesh;
		uint32_t m_ObjectID;

		// World matrices
		glm::mat4 m_World;
		//size_t m_NumInstances;
		//
		// Material
		MaterialData* m_pMaterial;
		LayerMask m_LayerMask;
	};
}
