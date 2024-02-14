#include "MaterialEditor.h"
#include "PropertyDrawer.h"
#include "EditorShaderProcessor.h"
#include "AssetPicker.h"
#include "Selection.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"
#include "EditorShaderData.h"
#include "EditorMaterialManager.h"

#include <imgui.h>
#include <ResourceType.h>
#include <PropertySerializer.h>
#include <ShaderSourceData.h>
#include <GLORY_YAML.h>
#include <AssetDatabase.h>
#include <IconsFontAwesome6.h>
#include <EditorUI.h>

namespace Glory::Editor
{
	MaterialEditor::MaterialEditor() {}

	MaterialEditor::~MaterialEditor() {}

	bool MaterialEditor::OnGUI()
	{
		YAMLResource<MaterialData>* pMaterial = (YAMLResource<MaterialData>*)m_pTarget;
		MaterialData* pMaterialData = EditorApplication::GetInstance()->GetMaterialManager().GetMaterial(pMaterial->GetUUID());

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
			Undo::StartRecord("Material Property", pMaterial->GetUUID());
			change = PropertiesGUI(pMaterial, pMaterialData);
			ImGui::TreePop();
			Undo::StopRecord();
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
		{
			EditorAssetDatabase::SetAssetDirty(pMaterial);
			pMaterial->SetDirty(true);
		}
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

	bool MaterialEditor::ShaderGUI(YAMLResource<MaterialData>* pMaterial)
	{
		bool change = false;

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
				ImGui::PushID((int)row_n);
				ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);

				auto shader = shaders[row_n]["UUID"];
				const UUID shaderID = shader.As<uint64_t>();
				ShaderSourceData* pShaderSourceData = EditorShaderProcessor::GetShaderSource(shaderID);
				if (!pShaderSourceData)
				{
					ImGui::TableSetColumnIndex(0);
					ImGui::Selectable(std::to_string(row_n).c_str(), false, selectableFlags, ImVec2(0, rowHeight));
					if (ImGui::TableSetColumnIndex(1))
						ImGui::TextColored({ 1,0,0,1 }, "UNKNOWN");
					if (ImGui::TableSetColumnIndex(2))
						ImGui::TextColored({1,0,0,1}, "Shader not yet loaded");
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
			EditorApplication::GetInstance()->GetMaterialManager().RemoveShaderFromMaterial(pMaterial->GetUUID(), toRemoveShaderIndex);
			change = true;
		}

		UUID addShaderID = 0;
		if (AssetPicker::ResourceButton("Add Shader", width, ResourceTypes::GetHash<ShaderSourceData>(), &addShaderID, false))
		{
			if (!EditorAssetDatabase::AssetExists(addShaderID)) return change;
			EditorApplication::GetInstance()->GetMaterialManager().AddShaderToMaterial(pMaterial->GetUUID(), addShaderID);
			change = true;
		}

		return change;
	}

	bool MaterialEditor::PropertiesGUI(YAMLResource<MaterialData>* pMaterial, MaterialData* pMaterialData)
	{
		EditorMaterialManager& materialManager = EditorApplication::GetInstance()->GetMaterialManager();
		Serializers& serializers = EditorApplication::GetInstance()->GetEngine()->GetSerializers();

		bool change = false;

		Utils::YAMLFileRef& file = **pMaterial;
		auto shaders = file["Shaders"];
		auto properties = file["Properties"];

		std::vector<EditorShaderData*> compiledShaders;

		static const uint32_t textureDataHash = ResourceTypes::GetHash<TextureData>();
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
				size_t materialPropertyIndex = 0;
				if (!pMaterialData->GetPropertyInfoIndex(materialManager, sampler, materialPropertyIndex))
					continue;
				MaterialPropertyInfo* pMaterialProperty = pMaterialData->GetPropertyInfoAt(materialManager, materialPropertyIndex);

				auto prop = properties[sampler];
				if (!prop.Exists()) {
					prop["DisplayName"].Set(sampler);
					prop["TypeHash"].Set(textureDataHash);
					prop["Value"].Set(0);
				}

				auto propValue = prop["Value"];
				PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(ST_Asset);

				ImGui::PushID(sampler.data());
				change |= pPropertyDrawer->Draw(file, propValue.Path(), pMaterialProperty->TypeHash(), pMaterialProperty->Flags());
				ImGui::PopID();

				/* Deserialize new value into resources array */
				if (change)
				{
					const UUID newUUID = propValue.As<uint64_t>();
					pMaterialData->SetTexture(materialManager, sampler, newUUID);
				}
			}

			for (size_t j = 0; j < pEditorShader->m_PropertyInfos.size(); ++j)
			{
				EditorShaderData::PropertyInfo& info = pEditorShader->m_PropertyInfos[j];
				size_t materialPropertyIndex = 0;
				if (!pMaterialData->GetPropertyInfoIndex(materialManager, info.m_Name, materialPropertyIndex))
					continue;
				MaterialPropertyInfo* pMaterialProperty = pMaterialData->GetPropertyInfoAt(materialManager, materialPropertyIndex);

				const size_t index = j + pEditorShader->m_SamplerNames.size();
				auto prop = properties[info.m_Name];

				if (!prop.Exists()) {
					prop["ShaderName"].Set(info.m_Name);
					prop["TypeHash"].Set(info.m_TypeHash);
				}

				ImGui::PushID(info.m_Name.data());
				auto propValue = prop["Value"];
				change |= PropertyDrawer::DrawProperty(file, propValue.Path(), info.m_TypeHash, info.m_TypeHash, pMaterialProperty->Flags());
				ImGui::PopID();

				/* Deserialize new value into buffer */
				if (change)
				{
					serializers.DeserializeProperty(pMaterialData->GetBufferReference(materialManager),
						pMaterialProperty->TypeHash(), pMaterialProperty->Offset(), pMaterialProperty->Size(), propValue.Node());
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
}
