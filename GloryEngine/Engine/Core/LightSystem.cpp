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
		light.type = pComponent.m_Type;
		light.direction = transform.MatTransform[2];
		light.color = glm::vec4(pComponent.m_Color, pComponent.m_Intensity);
		light.data.x = pComponent.m_Type == LightType::Spot ?
			std::cos(glm::radians(pComponent.m_Inner*0.5f)) : pComponent.m_Inner;
		light.data.y = pComponent.m_Type == LightType::Spot ?
			std::cos(glm::radians(pComponent.m_Outer *0.5f)) : pComponent.m_Outer;
		light.data.z = pComponent.m_Range;
		light.data.w = pComponent.m_FalloffExponent;
		light.shadowsEnabled = pComponent.m_Shadows.m_Enable ? 1 : 0;
		light.shadowBias = pComponent.m_Shadows.m_Bias;

		glm::mat4 lightView = glm::inverse(transform.MatTransform);
		glm::mat4 lightProjection;

		switch (light.type)
		{
		case LightType::Spot:
		{
			const float outerRadius = pComponent.m_Range * std::tan(glm::radians(pComponent.m_Outer / 2.0f));
			lightProjection = glm::perspective(glm::radians(pComponent.m_Outer), 1.0f, 0.001f, pComponent.m_Range + outerRadius*2.0f + 1.0f);
			break;
		}
		case LightType::Point:
			lightProjection = glm::perspective(glm::radians(45.0f), 1.0f, 0.001f, pComponent.m_Outer);
			break;
		case LightType::Sun:
			lightProjection = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, 0.01f, 3000.0f);
			break;
		default:
			break;
		}

		glm::mat4 lightSpace = lightProjection*lightView;
		pEngine->GetMainModule<RendererModule>()->Submit(std::move(light), std::move(lightSpace), pScene->GetEntityUUID(entity));
	}

	LightSystem::LightSystem()
	{
	}

	LightSystem::~LightSystem()
	{
	}
}
