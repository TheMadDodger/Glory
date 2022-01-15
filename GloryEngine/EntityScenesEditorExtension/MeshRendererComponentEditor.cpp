#include "MeshRendererComponentEditor.h"
#include <imgui.h>
#include <Selection.h>

namespace Glory::Editor
{
	MeshRendererComponentEditor::MeshRendererComponentEditor()
	{
	}

	MeshRendererComponentEditor::~MeshRendererComponentEditor()
	{
	}

	void MeshRendererComponentEditor::OnGUI()
	{
		MeshRenderer& meshRenderer = GetTargetComponent();

		if (ImGui::Button("Select material"))
		{
			Selection::SetActiveObject(meshRenderer.m_pMaterials[0]);
		}
	}

	std::string MeshRendererComponentEditor::Name()
	{
		return "Mesh Renderer";
	}
}
