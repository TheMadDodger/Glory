#include "LightComponentEditor.h"
#include "EditorApplication.h"

#include <Engine.h>
#include <Renderer.h>

namespace Glory::Editor
{
	LightComponentEditor::LightComponentEditor()
	{
	}

	LightComponentEditor::~LightComponentEditor()
	{
	}

	void LightComponentEditor::Initialize()
	{
		EntityComponentEditor::Initialize();
	}

	bool LightComponentEditor::OnGUI()
	{
		const bool change = EntityComponentEditor::OnGUI();
		Transform& transform = m_pComponentObject->GetRegistry()->GetComponent<Transform>(m_pComponentObject->EntityID());
		LightComponent& light = GetTargetComponent();
		Renderer* pRenderer = EditorApplication::GetInstance()->GetEngine()->ActiveRenderer();

		const glm::vec3 dir = transform.MatTransform[2];
		const glm::vec3 pos = transform.MatTransform[3];

		switch (light.m_Type)
		{
		case LightType::Sun:
		{
			const float radius = 1.0f;
			const glm::vec3 startPoint{};
			const glm::vec3 endPoint = startPoint - glm::vec3{ 0.0f, 0.0f, radius*10.0f };
			const glm::vec3 circlePoint = endPoint + glm::vec3{ 0.0f, 0.0f, radius*2.0f };
			pRenderer->DrawLine(transform.MatTransform, startPoint, endPoint, glm::vec4(1.0f, 0.894f, 0.518f, 1.0f));
			pRenderer->DrawLineCircle(transform.MatTransform, circlePoint, radius, Renderer::CircleUp::z, glm::vec4(1.0f, 0.894f, 0.518f, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, endPoint, circlePoint + glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.894f, 0.518f, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, endPoint, circlePoint + glm::vec3(radius, 0.0f, 0.0f), glm::vec4(1.0f, 0.894f, 0.518f, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, endPoint, circlePoint + glm::vec3(-radius, 0.0f, 0.0f), glm::vec4(1.0f, 0.894f, 0.518f, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, endPoint, circlePoint + glm::vec3(0.0f, radius, 0.0f), glm::vec4(1.0f, 0.894f, 0.518f, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, endPoint, circlePoint + glm::vec3(0.0f, -radius, 0.0f), glm::vec4(1.0f, 0.894f, 0.518f, 1.0f));
			break;
		}
		case LightType::Point:
			pRenderer->DrawLineSphere(glm::identity<glm::mat4>(), pos, light.m_Inner, glm::vec4(light.m_Color, 1.0f));
			pRenderer->DrawLineSphere(glm::identity<glm::mat4>(), pos, light.m_Outer, glm::vec4(light.m_Color*0.5f, 1.0f));
			break;
		case LightType::Spot:
		{
			const glm::vec3 startPoint{};
			const glm::vec3 endPoint = startPoint - glm::vec3{ 0.0f, 0.0f, light.m_Range };
			const float innerRadius = light.m_Range*std::tan(glm::radians(light.m_Inner/2.0f));
			const float outerRadius = light.m_Range*std::tan(glm::radians(light.m_Outer/2.0f));

			pRenderer->DrawLineCircle(transform.MatTransform, endPoint, innerRadius, Renderer::CircleUp::z, glm::vec4(light.m_Color, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, startPoint, endPoint + glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(light.m_Color, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, startPoint, endPoint + glm::vec3(innerRadius, 0.0f, 0.0f), glm::vec4(light.m_Color, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, startPoint, endPoint + glm::vec3(-innerRadius, 0.0f, 0.0f), glm::vec4(light.m_Color, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, startPoint, endPoint + glm::vec3(0.0f, innerRadius, 0.0f), glm::vec4(light.m_Color, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, startPoint, endPoint + glm::vec3(0.0f, -innerRadius, 0.0f), glm::vec4(light.m_Color, 1.0f));

			pRenderer->DrawLineCircle(transform.MatTransform, endPoint, outerRadius, Renderer::CircleUp::z, glm::vec4(light.m_Color*0.5f, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, startPoint, endPoint + glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(light.m_Color*0.5f, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, startPoint, endPoint + glm::vec3(outerRadius, 0.0f, 0.0f), glm::vec4(light.m_Color*0.5f, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, startPoint, endPoint + glm::vec3(-outerRadius, 0.0f, 0.0f), glm::vec4(light.m_Color*0.5f, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, startPoint, endPoint + glm::vec3(0.0f, outerRadius, 0.0f), glm::vec4(light.m_Color*0.5f, 1.0f));
			pRenderer->DrawLine(transform.MatTransform, startPoint, endPoint + glm::vec3(0.0f, -outerRadius, 0.0f), glm::vec4(light.m_Color*0.5f, 1.0f));

			pRenderer->DrawLineSphere(transform.MatTransform, endPoint, innerRadius, glm::vec4(light.m_Color, 1.0f));
			pRenderer->DrawLineSphere(transform.MatTransform, endPoint, outerRadius, glm::vec4(light.m_Color*0.5f, 1.0f));
			break;
		}
		default:
			break;
		}
		return change;
	}

	std::string LightComponentEditor::Name()
	{
		return "Light";
	}
}
