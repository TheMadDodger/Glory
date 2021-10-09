#pragma once
#include <glm/glm.hpp>
#include "ModelData.h"
#include "MaterialData.h"

namespace Glory
{
	struct UniformBufferObjectTest
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	struct RenderData
	{
	public:
		// Mesh
		ModelData* m_pModel;
		size_t m_MeshIndex;

		// TEMPORARY
		UniformBufferObjectTest m_UBO;

		// World matrices
		const glm::mat4 m_World;
		//size_t m_NumInstances;

		// Camera ?
		glm::mat4 m_ViewProjection;
		
		// Material
		MaterialData* m_pMaterial;
	};
}
