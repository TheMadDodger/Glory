#include "LightSystem.h"
#include <Game.h>
#include <Engine.h>
#include <PropertyFlags.h>
#include <SerializedPropertyManager.h>
#include <EntityRegistry.h>

namespace Glory
{
	void LightSystem::OnDraw(GloryECS::EntityRegistry* pRegistry, EntityID entity, LightComponent& pComponent)
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

	LightSystem::LightSystem()
	{
	}

	LightSystem::~LightSystem()
	{
	}
}
