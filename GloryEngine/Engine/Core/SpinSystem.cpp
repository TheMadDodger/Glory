#include "SpinSystem.h"
#include "GameTime.h"
#include "Engine.h"
#include "SceneManager.h"
#include "GSCene.h"

#include <chrono>
#include <glm/gtx/matrix_decompose.hpp>
#include <EntityRegistry.h>

namespace Glory
{
	void SpinSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, Spin& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();

		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		pComponent.m_Time += pEngine->Time().GetDeltaTime()*pComponent.m_Speed;
		transform.Rotation = glm::quat(glm::vec3(0.0f, pComponent.m_Time * glm::radians(90.0f), 0.0f));
	}
}
