#include "LightComponentEditor.h"
#include <imgui.h>

namespace Glory::Editor
{
	LightComponentEditor::LightComponentEditor()
	{
	}

	LightComponentEditor::~LightComponentEditor()
	{
	}

	void LightComponentEditor::OnGUI()
	{
		LightComponent& light = GetTargetComponent();

		ImGui::InputFloat4("Color", (float*)&light.m_Color);
		ImGui::InputFloat("Intensity", (float*)&light.m_Intensity);
		ImGui::InputFloat("Range", (float*)&light.m_Range);
	}

	std::string LightComponentEditor::Name()
	{
		return "Light";
	}
}
