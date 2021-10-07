#pragma once
#include <glm/glm.hpp>
#include "ModelData.h"
#include "Material.h"

namespace Glory
{
	struct RenderData
	{
	public:
		// Mesh
		ModelData* m_pModel;
		size_t m_MeshIndex;

		// World matrices
		const glm::mat4* m_World;
		size_t m_NumInstances;

		// Camera ?
		glm::mat4 m_ViewProjection;
		
		// Material ?
		Material* m_pMaterial;
	};
}
