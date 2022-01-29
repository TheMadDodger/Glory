#include "MaterialInstanceEditor.h"
#include "PropertyDrawer.h"
#include <imgui.h>

namespace Glory::Editor
{
	MaterialInstanceEditor::MaterialInstanceEditor() {}

	MaterialInstanceEditor::~MaterialInstanceEditor() {}

	void MaterialInstanceEditor::OnGUI()
	{
		MaterialInstanceData* pMaterial = (MaterialInstanceData*)m_pTarget;

		std::vector<MaterialPropertyData> properties;
		std::vector<bool> overrideStates;
		pMaterial->CopyProperties(properties);
		pMaterial->CopyOverrideStates(overrideStates);

		for (size_t i = 0; i < properties.size(); i++)
		{
			std::string label = "##override_" + std::to_string(i);

			bool enable = overrideStates[i];
			ImGui::Checkbox(label.data(), &enable);
			ImGui::BeginDisabled(!enable);
			ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
			MaterialPropertyData* pProperty = &properties[i];
			size_t hash = ResourceType::GetHash(pProperty->Type());
			PropertyDrawer::DrawProperty(pProperty->Name(), pProperty->Data(), pProperty->Flags());
			ImGui::EndDisabled();
			overrideStates[i] = enable;
		}

		pMaterial->PasteProperties(properties);
		pMaterial->PasteOverrideStates(overrideStates);
	}
}
