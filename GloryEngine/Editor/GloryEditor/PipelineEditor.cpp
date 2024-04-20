#include "PipelineEditor.h"
#include "EditorShaderProcessor.h"
#include "Selection.h"
#include "EditorApplication.h"
#include "EditorMaterialManager.h"
#include "AssetPicker.h"
#include "EditorAssetDatabase.h"
#include "EditorPipelineManager.h"

#include <GraphicsEnums.h>
#include <EditorUI.h>
#include <ShaderSourceData.h>
#include <GLORY_YAML.h>

namespace Glory::Editor
{
	PipelineEditor::PipelineEditor()
	{
	}

	PipelineEditor::~PipelineEditor()
	{
	}

	bool ShaderGUI(YAMLResource<PipelineData>* pPipeline)
	{
		bool change = false;

		static ImGuiTableFlags flags =
			ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody;

		Utils::YAMLFileRef& file = **pPipeline;
		auto shaders = file["Shaders"];

		const size_t shaderCount = shaders.Size();

		const float rowHeight = 25.0f;
		const float totalHeight = (shaderCount)*rowHeight + 30.0f;

		int toRemoveShaderIndex = -1;

		const float width = ImGui::GetContentRegionAvail().x;
		const float removeButtonWidth = 100.0f;

		if (ImGui::BeginTable("Loaded Shaders Table", 4, flags, ImVec2(0.0f, totalHeight), 0.0f))
		{
			ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 8.0f, 0);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 60.0f, 1);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, width - 100.0f - removeButtonWidth, 2);
			ImGui::TableSetupColumn("Remove?", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, removeButtonWidth, 3);

			ImGui::TableHeadersRow();

			static const ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

			for (size_t row_n = 0; row_n < shaderCount; ++row_n)
			{
				ImGui::PushID((int)row_n);
				ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);

				auto shader = shaders[row_n];
				const UUID shaderID = shader.As<uint64_t>();
				ShaderSourceData* pShaderSourceData = EditorShaderProcessor::GetShaderSource(shaderID);
				if (!pShaderSourceData)
				{
					ImGui::TableSetColumnIndex(0);
					ImGui::Selectable(std::to_string(row_n).c_str(), false, selectableFlags, ImVec2(0, rowHeight));
					if (ImGui::TableSetColumnIndex(1))
						ImGui::TextColored({ 1,0,0,1 }, "UNKNOWN");
					if (ImGui::TableSetColumnIndex(2))
						ImGui::TextColored({ 1,0,0,1 }, "Shader not yet loaded");
					if (ImGui::TableSetColumnIndex(3))
					{
						if (ImGui::Button("Remove", ImVec2(removeButtonWidth - 2.5f, rowHeight)))
						{
							toRemoveShaderIndex = (int)row_n;
						}
					}
					ImGui::PopID();
					continue;
				}


				ImGui::TableSetColumnIndex(0);

				if (ImGui::Selectable(std::to_string(row_n).c_str(), false, selectableFlags, ImVec2(0, rowHeight)) && ImGui::IsMouseDoubleClicked(0))
				{
					Selection::SetActiveObject(pShaderSourceData);
				}

				std::string name = pShaderSourceData->Name();
				ShaderType shaderType = pShaderSourceData->GetShaderType();
				if (name == "") name = "UNKNOWN SHADER";

				const std::string shaderTypeString = YAML::SHADERTYPE_TOFULLSTRING[shaderType];

				if (ImGui::TableSetColumnIndex(1))
					ImGui::TextUnformatted(shaderTypeString.c_str());

				if (ImGui::TableSetColumnIndex(2))
					ImGui::TextUnformatted(name.c_str());

				if (ImGui::TableSetColumnIndex(3) && ImGui::Button("Remove", ImVec2(removeButtonWidth - 2.5f, rowHeight)))
				{
					toRemoveShaderIndex = (int)row_n;
				}

				ImGui::PopID();
			}
			ImGui::EndTable();
		}

		if (toRemoveShaderIndex != -1)
		{
			EditorApplication::GetInstance()->GetPipelineManager().RemoveShaderFromPipeline(pPipeline->GetUUID(), toRemoveShaderIndex);
			change = true;
		}

		UUID addShaderID = 0;
		if (AssetPicker::ResourceButton("Add Shader", width, ResourceTypes::GetHash<ShaderSourceData>(), &addShaderID, false))
		{
			if (!EditorAssetDatabase::AssetExists(addShaderID)) return change;
			EditorApplication::GetInstance()->GetPipelineManager().AddShaderToPipeline(pPipeline->GetUUID(), addShaderID);
			change = true;
		}

		return change;
	}

	bool PipelineEditor::OnGUI()
	{
		YAMLResource<PipelineData>* pPipeline = (YAMLResource<PipelineData>*)m_pTarget;
		Utils::YAMLFileRef& file = **pPipeline;

		bool change = EditorUI::InputEnum<PipelineType>(file, "Type", { PipelineType::PT_Unknown, PipelineType::PT_Count });

		bool node = ImGui::TreeNodeEx("Loaded Shaders", ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen);
		if (node)
		{
			change |= ShaderGUI(pPipeline);
			ImGui::TreePop();
		}

		if (change)
		{
			EditorAssetDatabase::SetAssetDirty(pPipeline);
			pPipeline->SetDirty(true);
		}

		return change;
	}

	void PipelineEditor::Initialize()
	{
	}
}