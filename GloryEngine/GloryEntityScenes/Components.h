#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ModelData.h>
#include <MaterialData.h>

namespace Glory
{
	struct Transform
	{
		Transform();
		Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);

		glm::vec3 Position;
		glm::vec3 Rotation;
		glm::vec3 Scale;

		glm::mat4 MatTransform;
	};

	struct MeshFilter
	{
		MeshFilter(ModelData* pModelData) : m_pModelData(pModelData) {}
		ModelData* m_pModelData;
	};

	struct MeshRenderer
	{
		MeshRenderer(MaterialData* pMaterial) : m_pMaterials({ pMaterial }) {}
		MeshRenderer() : m_pMaterials(std::vector<MaterialData*>()) {}
		std::vector<MaterialData*> m_pMaterials;
	};
}
