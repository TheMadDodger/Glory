#include "TransformComponentEditor.h"
#include <imgui.h>

namespace Glory::Editor
{
	TransformComponentEditor::TransformComponentEditor()
	{
	}

	TransformComponentEditor::~TransformComponentEditor()
	{
	}

	void TransformComponentEditor::OnGUI()
	{
		Transform& transform = GetTargetComponent();

		ImGui::InputFloat3("Position", (float*)&transform.Position);
		ImGui::InputFloat3("Rotation", (float*)&transform.Rotation);
		ImGui::InputFloat3("Scale", (float*)&transform.Scale);
	}
	std::string TransformComponentEditor::Name()
	{
		return "Transform";
	}
}
