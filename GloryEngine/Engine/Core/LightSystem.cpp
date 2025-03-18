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
		LightData light;
		light.position = transform.MatTransform[3];
		light.position.w = (float)pComponent.m_Type;
		light.direction = transform.MatTransform[2];
		light.color = pComponent.m_Color;
		light.color.a = 1.0f;
		light.data.x = pComponent.m_Inner;
		light.data.y = pComponent.m_Outer;
		light.data.z = pComponent.m_Range;
		light.data.w = pComponent.m_Intensity;

		pEngine->GetMainModule<RendererModule>()->Submit(std::move(light));
	}

	LightSystem::LightSystem()
	{
	}

	LightSystem::~LightSystem()
	{
	}
}
