#include "LookAtSystem.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <EntityRegistry.h>

namespace Glory
{
	void LookAtSystem::OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, LookAt& pComponent)
	{
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		glm::mat4 matrix = glm::lookAt(pComponent.m_Eye, pComponent.m_Center, pComponent.m_Up);
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(matrix, transform.Scale, transform.Rotation, transform.Position, skew, perspective);
	}

	LookAtSystem::LookAtSystem()
	{
	}

	LookAtSystem::~LookAtSystem()
	{
	}
}
