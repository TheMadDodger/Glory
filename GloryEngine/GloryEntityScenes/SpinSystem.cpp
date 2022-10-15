#include "SpinSystem.h"
#include "Registry.h"
#include <chrono>
#include <glm/gtx/matrix_decompose.hpp>
#include <SerializedPropertyManager.h>
#include <GameTime.h>

namespace Glory
{
	void SpinSystem::OnUpdate(Registry* pRegistry, EntityID entity, Spin& pComponent)
	{
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		pComponent.m_Time += Time::GetDeltaTime<float, std::ratio<1, 1>>() * pComponent.m_Speed;
		transform.Rotation = glm::quat(glm::vec3(0.0f, pComponent.m_Time * glm::radians(90.0f), 0.0f));
	}

	void SpinSystem::OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, Spin& pComponent)
	{
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<float>>(uuid, std::string("Speed"), &pComponent.m_Speed, 0));
	}

	std::string SpinSystem::Name()
	{
		return "Spin";
	}
}
