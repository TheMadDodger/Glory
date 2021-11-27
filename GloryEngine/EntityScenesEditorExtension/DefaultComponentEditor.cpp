#include "DefaultComponentEditor.h"
#include <imgui.h>

namespace Glory::Editor
{
	DefaultComponentEditor::DefaultComponentEditor()
	{
	}

	DefaultComponentEditor::~DefaultComponentEditor()
	{
	}

	void DefaultComponentEditor::OnGUI()
	{
		ImGui::Text("Katarn stole the component!");
	}
}
