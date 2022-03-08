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

		std::vector<bool> overrideStates;
		pMaterial->CopyOverrideStates(overrideStates);

		std::vector<char>& buffer = pMaterial->GetBufferReference();

		size_t resourceCounter = 0;
		for (size_t i = 0; i < pMaterial->PropertyInfoCount(); i++)
		{
			std::string label = "##override_" + std::to_string(i);

			bool enable = overrideStates[i];
			MaterialPropertyInfo* info = pMaterial->GetPropertyInfoAt(i);

			ImGui::Checkbox(label.data(), &enable);
			ImGui::BeginDisabled(!enable);
			ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

			if (info->IsResource())
			{
				SerializedProperty serializedProperty = SerializedProperty(0, info->DisplayName(), SerializedType::ST_Asset, info->TypeHash(), pMaterial->GetResourcePointer(resourceCounter), info->Flags());
				PropertyDrawer::DrawProperty(&serializedProperty);
				++resourceCounter;
			}
			else PropertyDrawer::DrawProperty(info->DisplayName(), pMaterial->GetBufferReference(), info->TypeHash(), info->Offset(), info->Size(), info->Flags());

			ImGui::EndDisabled();
			overrideStates[i] = enable;
		}

		pMaterial->PasteOverrideStates(overrideStates);
	}
}
