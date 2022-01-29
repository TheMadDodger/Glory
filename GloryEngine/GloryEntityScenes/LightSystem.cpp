#include "LightSystem.h"
#include <Game.h>
#include <Engine.h>

namespace Glory
{
	void LightSystem::OnComponentAdded(Registry* pRegistry, EntityID entity, LightComponent& pComponent)
	{
	}

	void LightSystem::OnComponentRemoved(Registry* pRegistry, EntityID entity, LightComponent& pComponent)
	{
	}

	void LightSystem::OnUpdate(Registry* pRegistry, EntityID entity, LightComponent& pComponent)
	{
	}

	void LightSystem::OnDraw(Registry* pRegistry, EntityID entity, LightComponent& pComponent)
	{
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		PointLight pointLight;
		pointLight.enabled = 1;
		pointLight.position = glm::vec4(transform.Position, 1.0f);
		pointLight.color = pComponent.m_Color;
		pointLight.intensity = pComponent.m_Intensity;
		pointLight.range = pComponent.m_Range;

		Game::GetGame().GetEngine()->GetRendererModule()->Submit(pointLight);
	}
}
