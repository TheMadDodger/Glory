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

#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	PipelineEditor::PipelineEditor()
	{
	}

	PipelineEditor::~PipelineEditor()
	{
	}

	const char* GetPipelineError(YAMLResource<PipelineData>* pPipeline)
	{
		Utils::YAMLFileRef& file = **pPipeline;
		auto shaders = file["Shaders"];

		if (shaders.Size() == 0)
			return "The pipeline is empty";

		uint8_t shaderCounts[(size_t)ShaderType::ST_Count] = {};
		for (size_t i = 0; i < shaders.Size(); ++i)
		{
			const UUID shaderID = shaders[i].As<uint64_t>();
			ShaderSourceData* pShaderSourceData = EditorShaderProcessor::GetShaderSource(shaderID);
			if (!pShaderSourceData)
				return "Some shaders have not yet loaded.";
			const ShaderType& type = pShaderSourceData->GetShaderType();
			++shaderCounts[(size_t)type];
		}

		if (shaderCounts[(size_t)ShaderType::ST_Vertex] > 1)
			return "You can only have 1 vertex shader per pipeline.";

		if (shaderCounts[(size_t)ShaderType::ST_Fragment] > 1)
			return "You can only have 1 fragment shader per pipeline.";

		if (shaderCounts[(size_t)ShaderType::ST_Fragment] > 0 && shaderCounts[(size_t)ShaderType::ST_Vertex] == 0)
			return "A pipeline with a fragment shader must also have a vertex shader.";

		if (shaderCounts[(size_t)ShaderType::ST_Vertex] > 0 && shaderCounts[(size_t)ShaderType::ST_Fragment] == 0)
			return "A pipeline with a vertex shader must also have a fragment shader.";

		if (shaderCounts[(size_t)ShaderType::ST_Geomtery] > 0 &&
			(shaderCounts[(size_t)ShaderType::ST_Fragment] == 0 || shaderCounts[(size_t)ShaderType::ST_Vertex] == 0))
			return "A pipeline with a geometry shader must also have a vertex and fragment shader.";

		if (shaderCounts[(size_t)ShaderType::ST_Compute] > 0)
		{
			for (size_t i = 0; i < (size_t)ShaderType::ST_Count; ++i)
			{
				if (i == (size_t)ShaderType::ST_Compute) continue;
				if (shaderCounts[i] > 0)
					return "A pipeline with a compute shader should not have other shaders.";
			}
		}

		/* TODO: More checks */

		return nullptr;
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

		const char* error = GetPipelineError(pPipeline);
		if (error)
		{
			const float childHeight = ImGui::CalcTextSize("A").y * 6;
			ImGui::BeginChild("error", { 0.0f, childHeight }, true, ImGuiWindowFlags_MenuBar);
			if (ImGui::BeginMenuBar())
			{
				ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, ICON_FA_CIRCLE_EXCLAMATION);
				ImGui::SameLine();
				ImGui::Text(" Material Error");
				ImGui::EndMenuBar();
			}
			ImGui::Text("This material has the following errors:");
			ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "ERROR: %s", error);
			ImGui::Text("Using this material may have unexpected results and errors.");
			ImGui::EndChild();
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