#include "LightSystem.h"
#include <Game.h>
#include <Engine.h>
#include <PropertyFlags.h>

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

	void LightSystem::OnAcquireSerializedProperties(std::vector<SerializedProperty>& properties, LightComponent& pComponent)
	{
		glm::vec4 m_Color;
		float m_Intensity;
		float m_Range;

		properties.push_back(BasicTemplatedSerializedProperty("Color", &pComponent.m_Color, Vec4Flags::Color));
		properties.push_back(BasicTemplatedSerializedProperty("Intensity", &pComponent.m_Intensity));
		properties.push_back(BasicTemplatedSerializedProperty("Range", &pComponent.m_Range));
	}

	std::string LightSystem::Name()
	{
		return "Light";
	}
}
