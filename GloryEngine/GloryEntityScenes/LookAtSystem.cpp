#include "LookAtSystem.h"
#include "Registry.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <SerializedPropertyManager.h>

namespace Glory
{
	void LookAtSystem::OnUpdate(Registry* pRegistry, EntityID entity, LookAt& pComponent)
	{
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		glm::mat4 matrix = glm::lookAt(pComponent.m_Eye, pComponent.m_Center, pComponent.m_Up);
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(matrix, transform.Scale, transform.Rotation, transform.Position, skew, perspective);
	}

	void LookAtSystem::OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, LookAt& pComponent)
	{
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<glm::vec3>>(uuid, std::string("Eye Position"), 0, &pComponent.m_Eye));
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<glm::vec3>>(uuid, std::string("Target Center"), 0, &pComponent.m_Center));
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<glm::vec3>>(uuid, std::string("Up Vector"), 0, &pComponent.m_Up));
	}

	std::string LookAtSystem::Name()
	{
		return "Look At";
	}
}
