#include "LightSystem.h"
#include <Game.h>
#include <Engine.h>
#include <PropertyFlags.h>
#include <SerializedPropertyManager.h>

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

	void LightSystem::OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, LightComponent& pComponent)
	{
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<glm::vec4>>(uuid, std::string("Color"), &pComponent.m_Color, Vec4Flags::Color));
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<float>>(uuid, std::string("Intensity"), &pComponent.m_Intensity, 0));
		properties.push_back(SerializedPropertyManager::GetProperty<BasicTemplatedSerializedProperty<float>>(uuid, std::string("Range"), &pComponent.m_Range, 0));
	}

	std::string LightSystem::Name()
	{
		return "Light";
	}
}
