#include "SpinSystem.h"
#include <chrono>
#include <glm/gtx/matrix_decompose.hpp>
#include <GameTime.h>
#include <EntityRegistry.h>

namespace Glory
{
	void SpinSystem::OnUpdate(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, Spin& pComponent)
	{
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		pComponent.m_Time += Time::GetDeltaTime<float, std::ratio<1, 1>>() * pComponent.m_Speed;
		transform.Rotation = glm::quat(glm::vec3(0.0f, pComponent.m_Time * glm::radians(90.0f), 0.0f));
	}
}
