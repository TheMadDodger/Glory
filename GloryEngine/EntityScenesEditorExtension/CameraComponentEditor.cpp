#include "CameraComponentEditor.h"
#include <imgui.h>

namespace Glory::Editor
{
	CameraComponentEditor::CameraComponentEditor()
	{
	}

	CameraComponentEditor::~CameraComponentEditor()
	{
	}

	void CameraComponentEditor::OnGUI()
	{
		CameraComponent& camera = GetTargetComponent();

		ImGui::InputFloat("Half FOV", &camera.m_HalfFOV);
		ImGui::InputFloat("Near Plane", &camera.m_Near);
		ImGui::InputFloat("Far Plane", &camera.m_Far);
		ImGui::InputInt("Display Index", &camera.m_DisplayIndex);
		ImGui::InputInt("Priority", &camera.m_Priority);
		ImGui::InputFloat4("Clear Color", (float*)&camera.m_ClearColor);
	}

	std::string CameraComponentEditor::Name()
	{
		return "Camera";
	}
}
