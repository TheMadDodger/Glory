#include "MaterialInstanceEditor.h"
#include "PropertyDrawer.h"
#include "AssetPickerPopup.h"
#include <imgui.h>

namespace Glory::Editor
{
	MaterialInstanceEditor::MaterialInstanceEditor() {}

	MaterialInstanceEditor::~MaterialInstanceEditor() {}

	bool MaterialInstanceEditor::OnGUI()
	{
		MaterialInstanceData* pMaterial = (MaterialInstanceData*)m_pTarget;

		MaterialData* pBaseMaterial = pMaterial->GetBaseMaterial();
		ImGui::Text("Material: %s", pBaseMaterial ? pBaseMaterial->Name().c_str() : "None");
		ImGui::SameLine();
		if (ImGui::Button("Change"))
		{
			AssetPickerPopup::Open(ResourceType::GetHash<MaterialData>(), [&](Resource* pResource)
			{
				if (!pResource) return;
				MaterialInstanceData* pMaterial = (MaterialInstanceData*)m_pTarget;
				MaterialData* pBaseMaterial = (MaterialData*)pResource;
				pMaterial->SetBaseMaterial(pBaseMaterial);
			}, ResourceType::GetHash<MaterialInstanceData>());
		}

		if (!pMaterial->GetBaseMaterial()) return false;

		std::vector<bool> overrideStates;
		pMaterial->CopyOverrideStates(overrideStates);

		std::vector<char>& buffer = pMaterial->GetBufferReference();

		bool change = false;
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
				SerializedProperty serializedProperty = SerializedProperty(0, info->DisplayName(), SerializedType::ST_Asset, info->TypeHash(), pMaterial->GetResourceUUIDPointer(resourceCounter), info->Flags());
				change |= PropertyDrawer::DrawProperty(&serializedProperty);
				++resourceCounter;
			}
			else change |= PropertyDrawer::DrawProperty(info->DisplayName(), pMaterial->GetBufferReference(), info->TypeHash(), info->Offset(), info->Size(), info->Flags());

			ImGui::EndDisabled();
			overrideStates[i] = enable;
		}

		pMaterial->PasteOverrideStates(overrideStates);
		return change;
	}

	void MaterialInstanceEditor::Initialize()
	{
		MaterialInstanceData* pMaterial = (MaterialInstanceData*)m_pTarget;
		if (!pMaterial) return;
		pMaterial->ReloadProperties();
	}
}
