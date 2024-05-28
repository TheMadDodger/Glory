#include "LightSystem.h"

#include "Engine.h"
#include "GScene.h"
#include "SceneManager.h"
#include "RendererModule.h"
#include "PropertyFlags.h"

#include <EntityRegistry.h>

namespace Glory
{
	void LightSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, LightComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();

		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		PointLight pointLight;
		pointLight.enabled = 1;
		pointLight.position = transform.MatTransform[3];
		pointLight.color = pComponent.m_Color;
		pointLight.intensity = pComponent.m_Intensity;
		pointLight.range = pComponent.m_Range;

		pEngine->GetMainModule<RendererModule>()->Submit(std::move(pointLight));
	}

	LightSystem::LightSystem()
	{
	}

	LightSystem::~LightSystem()
	{
	}
}
