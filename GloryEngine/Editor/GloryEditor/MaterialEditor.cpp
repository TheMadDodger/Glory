#include "MaterialEditor.h"
#include "PropertyDrawer.h"
#include "EditorShaderProcessor.h"
#include "AssetPicker.h"
#include "Selection.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"
#include "EditorShaderData.h"

#include <imgui.h>
#include <ResourceType.h>
#include <GLORY_YAML.h>
#include <AssetDatabase.h>
#include <AssetManager.h>
#include <IconsFontAwesome6.h>
#include <EditorUI.h>

namespace Glory::Editor
{
	MaterialEditor::MaterialEditor() {}

	MaterialEditor::~MaterialEditor() {}

	bool MaterialEditor::OnGUI()
	{
		YAMLResource<MaterialData>* pMaterial = (YAMLResource<MaterialData>*)m_pTarget;

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

		const char* error = GetMaterialError(pMaterial);
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
			EditorAssetDatabase::SetAssetDirty(pMaterial);
		return change;
	}

	const char* MaterialEditor::GetMaterialError(YAMLResource<MaterialData>* pMaterial)
	{
		Utils::YAMLFileRef& file = **pMaterial;
		auto shaders = file["Shaders"];

		if (shaders.Size() == 0)
			return "The material is empty";

		uint8_t shaderCounts[(size_t)ShaderType::ST_Count] = {};
		for (size_t i = 0; i < shaders.Size(); ++i)
		{
			const UUID shaderID = shaders[i]["UUID"].As<uint64_t>();
			ShaderSourceData* pShaderSourceData = EditorShaderProcessor::GetShaderSource(shaderID);
			if (!pShaderSourceData)
				return "Some shaders have not yet loaded.";
			const ShaderType& type = pShaderSourceData->GetShaderType();
			++shaderCounts[(size_t)type];
		}

		if (shaderCounts[(size_t)ShaderType::ST_Vertex] > 1)
			return "You can only have 1 vertex shader per material.";

		if (shaderCounts[(size_t)ShaderType::ST_Fragment] > 1)
			return "You can only have 1 fragment shader per material.";

		if (shaderCounts[(size_t)ShaderType::ST_Fragment] > 0 && shaderCounts[(size_t)ShaderType::ST_Vertex] == 0)
			return "A material with a fragment shader must also have a vertex shader.";

		if (shaderCounts[(size_t)ShaderType::ST_Vertex] > 0 && shaderCounts[(size_t)ShaderType::ST_Fragment] == 0)
			return "A material with a vertex shader must also have a fragment shader.";

		if (shaderCounts[(size_t)ShaderType::ST_Geomtery] > 0 &&
			(shaderCounts[(size_t)ShaderType::ST_Fragment] == 0 || shaderCounts[(size_t)ShaderType::ST_Vertex] == 0))
			return "A material with a geometry shader must also have a vertex and fragment shader.";

		if (shaderCounts[(size_t)ShaderType::ST_Compute] > 0)
		{
			for (size_t i = 0; i < (size_t)ShaderType::ST_Count; ++i)
			{
				if (i == (size_t)ShaderType::ST_Compute) continue;
				if (shaderCounts[i] > 0)
					return "A material with a compute shader should not have other shaders.";
			}
		}

		/* TODO: More checks */

		return nullptr;
	}

	void MaterialEditor::ShaderGUI(YAMLResource<MaterialData>* pMaterial)
	{
		static ImGuiTableFlags flags =
			ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody;

		Utils::YAMLFileRef& file = **pMaterial;
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
				auto shader = shaders[row_n]["UUID"];
				const UUID shaderID = shader.As<uint64_t>();
				ShaderSourceData* pShaderSourceData = EditorShaderProcessor::GetShaderSource(shaderID);
				if (!pShaderSourceData)
				{
					ImGui::Selectable(std::to_string(row_n).c_str(), false, selectableFlags, ImVec2(0, rowHeight));
					ImGui::SameLine();
					ImGui::TextColored({1,0,0,1}, "Shader not yet loaded");
					continue;
				}

				std::string name = pShaderSourceData->Name();
				ShaderType shaderType = pShaderSourceData->GetShaderType();
				if (name == "") name = "UNKNOWN SHADER";

				const std::string shaderTypeString = YAML::SHADERTYPE_TOFULLSTRING[shaderType];

				std::string label = shaderTypeString + ": " + name;

				ImGui::PushID((int)row_n);
				ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);

				ImGui::TableSetColumnIndex(0);

				if (ImGui::Selectable(std::to_string(row_n).c_str(), false, selectableFlags, ImVec2(0, rowHeight)) && ImGui::IsMouseDoubleClicked(0))
				{
					Selection::SetActiveObject(pShaderSourceData);
				}

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
			shaders.Remove(toRemoveShaderIndex);
		}

		UUID addShaderID = 0;
		if (AssetPicker::ResourceButton("Add Shader", width, ResourceTypes::GetHash<ShaderSourceData>(), &addShaderID))
		{
			if (!EditorAssetDatabase::AssetExists(addShaderID)) return;
			MaterialData* pMaterial = (MaterialData*)m_pTarget;
			AssetManager& assets = EditorApplication::GetInstance()->GetEngine()->GetAssetManager();
			ShaderSourceData* pShaderSource = (ShaderSourceData*)assets.GetAssetImmediate(addShaderID);
			if (!pMaterial->AddShader(pShaderSource)) return;
			UpdateMaterial(pMaterial);
		}
	}

	bool MaterialEditor::PropertiesGUI(YAMLResource<MaterialData>* pMaterial)
	{
		bool change = false;

		Utils::YAMLFileRef& file = **pMaterial;
		auto shaders = file["Shaders"];
		auto properties = file["Properties"];

		std::vector<EditorShaderData*> compiledShaders;

		for (size_t i = 0; i < shaders.Size(); ++i)
		{
			const UUID shaderID = shaders[i]["UUID"].As<uint64_t>();
			EditorShaderData* pEditorShader = EditorShaderProcessor::GetEditorShader(shaderID);
			if (!pEditorShader)
			{
				ImGui::TextColored({ 1,0,0,1 }, "A shader has not yet compiled");
				return false;
			}
			
			for (size_t j = 0; j < pEditorShader->m_SamplerNames.size(); ++j)
			{
				const std::string& sampler = pEditorShader->m_SamplerNames[j];

				static const uint32_t imageDataHash = ResourceTypes::GetHash<ImageData>();

				auto prop = properties[sampler];
				if (!prop.Exists()) {
					prop["DisplayName"].Set(sampler);
					prop["TypeHash"].Set(imageDataHash);
					prop["Value"].Set(0);
				}
				PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(ST_Asset);
				UUID value = prop["Value"].As<uint64_t>();
				if (AssetPicker::ResourceDropdown(sampler, imageDataHash, &value))
				{
					change = true;
					prop["Value"].Set(value);
				}
			}

			for (size_t j = 0; j < pEditorShader->m_PropertyInfos.size(); ++j)
			{
				EditorShaderData::PropertyInfo& info = pEditorShader->m_PropertyInfos[j];

				const size_t index = j + pEditorShader->m_SamplerNames.size();
				auto prop = properties[info.m_Name];

				if (!prop.Exists()) {
					prop["ShaderName"].Set(info.m_Name);
					prop["TypeHash"].Set(info.m_TypeHash);
				}

				static const uint32_t f = ResourceTypes::GetHash<float>();
				static const uint32_t f2 = ResourceTypes::GetHash<glm::vec2>();
				static const uint32_t f3 = ResourceTypes::GetHash<glm::vec3>();
				static const uint32_t f4 = ResourceTypes::GetHash<glm::vec4>();

				ImGui::TextUnformatted(info.m_Name.data());
				ImGui::SameLine();

				auto propValue = prop["Value"];
				if (info.m_TypeHash == f)
				{
					if (!propValue.Exists())
						propValue.Set(0.0f);
					change |= EditorUI::InputFloat(file, propValue.Path());
				}
				else if (info.m_TypeHash == f2)
				{
					if (!propValue.Exists())
						propValue.Set(glm::vec2{ 0,0 });
					change |= EditorUI::InputFloat2(file, propValue.Path());
				}
				else if (info.m_TypeHash == f3)
				{
					if (!propValue.Exists())
						propValue.Set(glm::vec3{ 0,0,0 });
					change |= EditorUI::InputFloat3(file, propValue.Path());
				}
				else if (info.m_TypeHash == f4)
				{
					if (!propValue.Exists())
						propValue.Set(glm::vec4{ 1,1,1,1 });
					change |= EditorUI::InputColor(file, propValue.Path(), false);
				}
			}
		}

		if (change)
		{
			pMaterial->SetDirty(true);
			EditorAssetDatabase::SetAssetDirty(pMaterial);
		}
		return change;
	}

	void MaterialEditor::UpdateMaterial(MaterialData* pMaterial)
	{
		/*pMaterial->ClearProperties();
		for (size_t i = 0; i < pMaterial->ShaderCount(); i++)
		{
			ShaderSourceData* pShader = pMaterial->GetShaderAt(i);
			EditorShaderData* pShaderData = EditorShaderProcessor::GetShaderSource(pShader);
			if (!pShaderData) continue;
			pShaderData->LoadIntoMaterial(pMaterial);
		}
		EditorAssetDatabase::SetAssetDirty(pMaterial);*/
	}
}
