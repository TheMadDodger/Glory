#include "MaterialEditor.h"
#include "PropertyDrawer.h"
#include "EditorShaderProcessor.h"
#include <imgui.h>
#include <ResourceType.h>
#include <GLORY_YAML.h>
#include <Selection.h>
#include <AssetDatabase.h>
#include <AssetPickerPopup.h>
#include <AssetReferencePropertyTemplate.h>

namespace Glory::Editor
{
	MaterialEditor::MaterialEditor() {}

	MaterialEditor::~MaterialEditor() {}

	bool MaterialEditor::OnGUI()
	{
		MaterialData* pMaterial = (MaterialData*)m_pTarget;

		bool node = ImGui::TreeNodeEx("Loaded Shaders", ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen);
		if (node)
		{
			ShaderGUI(pMaterial);
			ImGui::TreePop();
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		bool change = false;
		node = ImGui::TreeNodeEx("Properties", ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen);
		if (node)
		{
			change = PropertiesGUI(pMaterial);
			ImGui::TreePop();
		}
		return change;
	}

	void MaterialEditor::ShaderGUI(MaterialData* pMaterial)
	{
		static ImGuiTableFlags flags =
			ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody;

		size_t shaderCount = pMaterial->ShaderCount();

		float rowHeight = 25.0f;
		float totalHeight = (shaderCount)*rowHeight + 30.0f;

		int toRemoveShaderIndex = -1;

		float width = ImGui::GetContentRegionAvail().x;
		float removeButtonWidth = 100.0f;

		if (ImGui::BeginTable("Loaded Shaders Table", 4, flags, ImVec2(0.0f, totalHeight), 0.0f))
		{
			ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 8.0f, 0);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 60.0f, 1);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, width - 100.0f - removeButtonWidth, 2);
			ImGui::TableSetupColumn("Remove?", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, removeButtonWidth, 3);

			ImGui::TableHeadersRow();

			for (size_t row_n = 0; row_n < shaderCount; row_n++)
			{
				ShaderSourceData* pShaderSourceData = pMaterial->GetShaderAt(row_n);
				std::string name = pShaderSourceData->Name();
				ShaderType shaderType = pMaterial->GetShaderTypeAt(row_n);
				if (name == "") name = "UNKNOWN SHADER";

				const std::string shaderTypeString = YAML::SHADERTYPE_TOFULLSTRING[shaderType];

				std::string label = shaderTypeString + ": " + name;

				ImGui::PushID(row_n);
				ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);

				ImGui::TableSetColumnIndex(0);
				ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

				if (ImGui::Selectable(std::to_string(row_n).c_str(), false, selectable_flags, ImVec2(0, rowHeight)) && ImGui::IsMouseDoubleClicked(0))
				{
					Selection::SetActiveObject(pShaderSourceData);
				}

				if (ImGui::TableSetColumnIndex(1))
					ImGui::TextUnformatted(shaderTypeString.c_str());

				if (ImGui::TableSetColumnIndex(2))
					ImGui::TextUnformatted(name.c_str());

				if (ImGui::TableSetColumnIndex(3) && ImGui::Button("Remove", ImVec2(removeButtonWidth - 2.5f, rowHeight)))
				{
					toRemoveShaderIndex = row_n;
				}

				ImGui::PopID();
			}
			ImGui::EndTable();
		}

		if (toRemoveShaderIndex != -1)
		{
			pMaterial->RemoveShaderAt((size_t)toRemoveShaderIndex);
			UpdateMaterial(pMaterial);
		}

		if (ImGui::Button("Add Shader", ImVec2(width, 0.0f)))
		{
			AssetPickerPopup::Open(ResourceType::GetHash<ShaderSourceData>(), [&](Resource* pResource)
			{
				if (!pResource) return;
				MaterialData* pMaterial = (MaterialData*)m_pTarget;
				ShaderSourceData* pShaderSource = (ShaderSourceData*)pResource;
				if (!pMaterial->AddShader(pShaderSource)) return;
				UpdateMaterial(pMaterial);

			});
		}
	}

	bool MaterialEditor::PropertiesGUI(MaterialData* pMaterial)
	{
		bool change = false;

		size_t resourceCounter = 0;
		for (size_t i = 0; i < pMaterial->PropertyInfoCount(); i++)
		{
			MaterialPropertyInfo* info = pMaterial->GetPropertyInfoAt(i);

			if (info->IsResource())
			{
				SerializedProperty serializedProperty = SerializedProperty(0, info->DisplayName(), SerializedType::ST_Asset, info->TypeHash(), pMaterial->GetResourceUUIDPointer(resourceCounter), info->Flags());
				change |= PropertyDrawer::DrawProperty(&serializedProperty);
				++resourceCounter;
			}
			else change |= PropertyDrawer::DrawProperty(info->DisplayName(), pMaterial->GetBufferReference(), info->TypeHash(), info->Offset(), info->Size(), info->Flags());
		}
		return change;
	}

	void MaterialEditor::UpdateMaterial(MaterialData* pMaterial)
	{
		pMaterial->ClearProperties();
		for (size_t i = 0; i < pMaterial->ShaderCount(); i++)
		{
			ShaderSourceData* pShader = pMaterial->GetShaderAt(i);
			EditorShaderData* pShaderData = EditorShaderProcessor::GetShaderSource(pShader);
			if (!pShaderData) continue;
			pShaderData->LoadIntoMaterial(pMaterial);
		}
		AssetDatabase::SaveAsset(pMaterial);
	}
}
