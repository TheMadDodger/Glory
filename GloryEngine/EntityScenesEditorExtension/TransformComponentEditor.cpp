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
		glm::vec3 euler = glm::eulerAngles(transform.Rotation) / 3.141592f * 180.0f;
		if (ImGui::InputFloat3("Rotation", (float*)&euler, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
		{
			transform.Rotation = glm::quat(euler * 3.141592f / 180.0f);
		}
		ImGui::InputFloat3("Scale", (float*)&transform.Scale);
	}
	std::string TransformComponentEditor::Name()
	{
		return "Transform";
	}
}
