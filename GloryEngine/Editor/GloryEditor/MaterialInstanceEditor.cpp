#include "MaterialInstanceEditor.h"
#include "PropertyDrawer.h"
#include "AssetPicker.h"
#include "AssetManager.h"
#include "EditorAssetDatabase.h"

#include <imgui.h>
#include <MaterialEditor.h>
#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	MaterialInstanceEditor::MaterialInstanceEditor() {}

	MaterialInstanceEditor::~MaterialInstanceEditor() {}

	bool MaterialInstanceEditor::OnGUI()
	{
		MaterialInstanceData* pMaterial = (MaterialInstanceData*)m_pTarget;

		MaterialData* pBaseMaterial = pMaterial->GetBaseMaterial();
		//ImGui::Text("Material: %s", pBaseMaterial ? pBaseMaterial->Name().c_str() : "None");
		//ImGui::SameLine();
		UUID baseMaterialID = pBaseMaterial ? pBaseMaterial->GetUUID() : UUID(0);
		bool change = false;
		if (AssetPicker::ResourceDropdown("Base Material", ResourceType::GetHash<MaterialData>(), &baseMaterialID))
		{
			change = true;
			MaterialData* pBaseMaterial = EditorAssetDatabase::AssetExists(baseMaterialID) ? (MaterialData*)AssetManager::GetAssetImmediate(baseMaterialID) : nullptr;
			pMaterial->SetBaseMaterial(pBaseMaterial);
		}

		if (!pMaterial->GetBaseMaterial())
		{
			DrawErrorWindow("No base material selected");
			return false;
		}

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
				PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(ST_Asset);
				change |= pPropertyDrawer->Draw(info->DisplayName(), pMaterial->GetResourceUUIDPointer(resourceCounter), info->TypeHash(), info->Flags());
				++resourceCounter;
			}
			else change |= PropertyDrawer::DrawProperty(info->DisplayName(), pMaterial->GetBufferReference(), info->TypeHash(), info->Offset(), info->Size(), info->Flags());

			ImGui::EndDisabled();
			overrideStates[i] = enable;
		}

		pMaterial->PasteOverrideStates(overrideStates);

		if (change)
			EditorAssetDatabase::SetAssetDirty(pMaterial);

		const char* error = MaterialEditor::GetMaterialError(pMaterial);
		DrawErrorWindow(error);

		return change;
	}

	void MaterialInstanceEditor::Initialize()
	{
		MaterialInstanceData* pMaterial = (MaterialInstanceData*)m_pTarget;
		if (!pMaterial) return;
		pMaterial->ReloadProperties();
	}

	void MaterialInstanceEditor::DrawErrorWindow(const char* error)
	{
		if (!error) return;
		const float childHeight = ImGui::CalcTextSize("A").y * 6;
		ImGui::BeginChild("error", { 0.0f, childHeight }, true, ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar())
		{
			ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, ICON_FA_CIRCLE_EXCLAMATION);
			ImGui::SameLine();
			ImGui::Text(" Material Error");
			ImGui::EndMenuBar();
		}
		ImGui::Text("The selected base material has the following errors:");
		ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "ERROR: %s", error);
		ImGui::Text("Using this material may have unexpected results and errors.");
		ImGui::EndChild();
	}
}
