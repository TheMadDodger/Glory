#include "PipelineEditor.h"
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
#include <YAML_GLM.h>

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
			ShaderSourceData* pShaderSourceData = EditorPipelineManager::GetShaderSource(shaderID);
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
				ShaderSourceData* pShaderSourceData = EditorPipelineManager::GetShaderSource(shaderID);
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
		const PipelineType pipelineType = file["Type"].AsEnum<PipelineType>(PipelineType::PT_Phong);

		ImGui::PushID("Shaders");
		if (EditorUI::Header("Shaders"))
		{
			change |= ShaderGUI(pPipeline);
		}
		ImGui::PopID();

		PipelineData* pPipelineData = EditorApplication::GetInstance()->
			GetPipelineManager().GetPipelineData(pPipeline->GetUUID());

		ImGui::Spacing();
		ImGui::PushID("Features");
		if (EditorUI::Header("Features"))
		{
			auto features = file["Features"];
			if (!features.Exists() || !features.IsMap())
				features.SetMap();

			for (size_t i = 0; pPipelineData && i < pPipelineData->FeatureCount(); ++i)
			{
				std::string_view featureName = pPipelineData->FeatureName(i);
				auto feature = features[featureName];
				if (!feature.Exists() || !feature.IsScalar())
					feature.Set(pPipelineData->FeatureEnabled(i));
				if (EditorUI::CheckBox(file, feature.Path()))
				{
					change = true;
					EditorApplication::GetInstance()->GetPipelineManager().
						SetPipelineFeatureEnabled(pPipeline->GetUUID(), featureName, feature.As<bool>());
				}
			}

			if (pPipelineData && pPipelineData->FeatureCount() == 0)
			{
				ImGui::TextUnformatted("This pipeline has no features.");
			}
		}
		ImGui::PopID();

		bool settingsChanged = false;
		if (pipelineType != PipelineType::PT_Compute)
		{
			ImGui::Spacing();
			ImGui::PushID("Settings");
			if (EditorUI::Header("Settings"))
			{
				auto settings = file["Settings"];
				if (!settings.Exists() || !settings.IsMap())
					settings.SetMap();

				ImGui::PushID("General");
				if (EditorUI::HeaderLight("General"))
				{
					settingsChanged |= EditorUI::InputEnum<CullFace>(file, settings["CullFace"].Path());
					settingsChanged |= EditorUI::InputEnum<PrimitiveType>(file, settings["PrimitiveType"].Path());
					settingsChanged |= EditorUI::CheckBoxFlags(file, settings["ColorWriteMask"].Path(), { "r", "g", "b", "a" }, { 1, 2, 4, 8 });
					settingsChanged |= EditorUI::CheckBox(file, settings["StencilTestEnabled"].Path());
				}
				ImGui::PopID();

				ImGui::PushID("Depth");
				if (EditorUI::HeaderLight("Depth"))
				{
					settingsChanged |= EditorUI::CheckBox(file, settings["DepthTestEnabled"].Path());
					settingsChanged |= EditorUI::CheckBox(file, settings["DepthWriteEnabled"].Path());
					settingsChanged |= EditorUI::InputEnum<CompareOp>(file, settings["DepthCompareOp"].Path());
				}
				ImGui::PopID();

				ImGui::PushID("Stencil");
				if (EditorUI::HeaderLight("Stencil"))
				{
					settingsChanged |= EditorUI::InputEnum<CompareOp>(file, settings["StencilCompareOp"].Path());
					settingsChanged |= EditorUI::InputEnum<Func>(file, settings["StencilFailOp"].Path());
					settingsChanged |= EditorUI::InputEnum<Func>(file, settings["StencilDepthFailOp"].Path());
					settingsChanged |= EditorUI::InputEnum<Func>(file, settings["StencilPassOp"].Path());
					settingsChanged |= EditorUI::InputUInt(file, settings["StencilCompareMask"].Path(), 0, 255, 1);
					settingsChanged |= EditorUI::InputUInt(file, settings["StencilWriteMask"].Path(), 0, 255, 1);
					settingsChanged |= EditorUI::InputUInt(file, settings["StencilReference"].Path(), 0, 255, 1);
				}
				ImGui::PopID();

				ImGui::PushID("Blending");
				if (EditorUI::HeaderLight("Blending"))
				{
					settingsChanged |= EditorUI::CheckBox(file, settings["BlendEnabled"].Path());
					settingsChanged |= EditorUI::InputEnum<BlendFactor>(file, settings["SrcColorBlendFactor"].Path());
					settingsChanged |= EditorUI::InputEnum<BlendFactor>(file, settings["DrcColorBlendFactor"].Path());
					settingsChanged |= EditorUI::InputEnum<BlendOp>(file, settings["ColorBlendOp"].Path());
					settingsChanged |= EditorUI::InputEnum<BlendFactor>(file, settings["SrcAlphaBlendFactor"].Path());
					settingsChanged |= EditorUI::InputEnum<BlendFactor>(file, settings["DstAlphaBlendFactor"].Path());
					settingsChanged |= EditorUI::InputEnum<BlendOp>(file, settings["AlphaBlendOp"].Path());
					settingsChanged |= EditorUI::InputFloat4(file, settings["BlendConstants"].Path());
				}
				ImGui::PopID();
			}
			ImGui::PopID();
		}
		change |= settingsChanged;
		if (settingsChanged && pPipelineData)
		{
			auto settings = file["Settings"];
			pPipelineData->GetCullFace() = settings["CullFace"].AsEnum<CullFace>(CullFace::Back);
			pPipelineData->GetPrimitiveType() = settings["PrimitiveType"].AsEnum<PrimitiveType>(PrimitiveType::Triangles);
			pPipelineData->SetDepthTestEnabled(settings["DepthTestEnabled"].As<bool>(true));
			pPipelineData->SetDepthWriteEnabled(settings["DepthWriteEnabled"].As<bool>(true));
			pPipelineData->GetDepthCompareOp() = settings["DepthCompareOp"].AsEnum<CompareOp>(CompareOp::OP_Less);
			pPipelineData->SetColorWriteMask(settings["ColorWriteMask"].As<uint8_t>(uint8_t(15)));
			pPipelineData->SetStencilTestEnabled(settings["StencilTestEnabled"].As<bool>(false));
			pPipelineData->GetStencilCompareOp() = settings["StencilCompareOp"].AsEnum<CompareOp>(CompareOp::OP_Always);
			pPipelineData->GetStencilFailOp() = settings["StencilFailOp"].AsEnum<Func>(Func::OP_Zero);
			pPipelineData->GetStencilDepthFailOp() = settings["StencilDepthFailOp"].AsEnum<Func>(Func::OP_Zero);
			pPipelineData->GetStencilPassOp() = settings["StencilPassOp"].AsEnum<Func>(Func::OP_Zero);
			pPipelineData->SetStencilCompareMask(settings["StencilCompareMask"].As<uint8_t>(0xFF));
			pPipelineData->SetStencilWriteMask(settings["StencilWriteMask"].As<uint8_t>(0x00));
			pPipelineData->SetStencilReference(settings["StencilReference"].As<uint8_t>(0x00));
			pPipelineData->SetBlendEnabled(settings["BlendEnabled"].As<bool>(true));
			pPipelineData->SrcColorBlendFactor() = settings["SrcColorBlendFactor"].AsEnum<BlendFactor>(BlendFactor::One);
			pPipelineData->DstColorBlendFactor() = settings["DrcColorBlendFactor"].AsEnum<BlendFactor>(BlendFactor::Zero);
			pPipelineData->ColorBlendOp() = settings["ColorBlendOp"].AsEnum<BlendOp>(BlendOp::Add);
			pPipelineData->SrcAlphaBlendFactor() = settings["SrcAlphaBlendFactor"].AsEnum<BlendFactor>(BlendFactor::One);
			pPipelineData->DstAlphaBlendFactor() = settings["DstAlphaBlendFactor"].AsEnum<BlendFactor>(BlendFactor::Zero);
			pPipelineData->AlphaBlendOp() = settings["AlphaBlendOp"].AsEnum<BlendOp>(BlendOp::Add);
			pPipelineData->BlendConstants() = settings["BlendConstants"].As<glm::vec4>(glm::vec4{});
			pPipelineData->SettingsDirty() = true;
		}

		ImGui::Spacing();
		const char* error = GetPipelineError(pPipeline);
		if (error)
		{
			const float childHeight = ImGui::CalcTextSize("A").y*6;
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