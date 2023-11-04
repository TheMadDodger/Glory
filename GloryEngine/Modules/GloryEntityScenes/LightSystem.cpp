#include "LightSystem.h"

#include <Engine.h>
#include <RendererModule.h>
#include <PropertyFlags.h>
#include <EntityRegistry.h>

namespace Glory
{
	void LightSystem::OnDraw(Glory::Utils::ECS::EntityRegistry* pRegistry, EntityID entity, LightComponent& pComponent)
	{
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		PointLight pointLight;
		pointLight.enabled = 1;
		pointLight.position = transform.MatTransform[3];
		pointLight.color = pComponent.m_Color;
		pointLight.intensity = pComponent.m_Intensity;
		pointLight.range = pComponent.m_Range;

		Game::GetGame().GetEngine()->GetMainModule<RendererModule>()->Submit(pointLight);
	}

	LightSystem::LightSystem()
	{
	}

	LightSystem::~LightSystem()
	{
	}
}
