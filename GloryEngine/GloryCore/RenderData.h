#pragma once
#include <glm/glm.hpp>
#include "LayerMask.h"
#include "ModelData.h"
#include "MaterialData.h"

namespace Glory
{
	struct UniformBufferObjectTest
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
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
