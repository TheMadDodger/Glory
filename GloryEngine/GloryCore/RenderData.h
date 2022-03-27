#pragma once
#include <glm/glm.hpp>
#include "LayerMask.h"
#include "ModelData.h"
#include "MaterialData.h"

namespace Glory
{
	struct ModelViewProjection
	{
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
	};

	struct RenderData
	{
	public:
		// Mesh
		ModelData* m_pModel;
		size_t m_MeshIndex;

		// World matrices
		glm::mat4 m_World;
		//size_t m_NumInstances;
		
		// Material
		MaterialData* m_pMaterial;
		LayerMask m_LayerMask;
	};
}
