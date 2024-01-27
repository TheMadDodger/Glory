#include "MaterialInstanceEditor.h"
#include "PropertyDrawer.h"
#include "AssetPicker.h"
#include "AssetManager.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"
#include "EditorShaderProcessor.h"
#include "EditorShaderData.h"
#include "EditorUI.h"

#include <imgui.h>
#include <MaterialEditor.h>
#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	MaterialInstanceEditor::MaterialInstanceEditor() {}

	MaterialInstanceEditor::~MaterialInstanceEditor() {}

	bool MaterialInstanceEditor::OnGUI()
	{
		YAMLResource<MaterialInstanceData>* pMaterial = (YAMLResource<MaterialInstanceData>*)m_pTarget;

		Utils::YAMLFileRef& file = **pMaterial;
		auto baseMaterial = file["BaseMaterial"];

		//MaterialData* pBaseMaterial = pMaterial->GetBaseMaterial();
		//ImGui::Text("Material: %s", pBaseMaterial ? pBaseMaterial->Name().c_str() : "None");
		//ImGui::SameLine();
		UUID baseMaterialID = baseMaterial.As<uint64_t>();
		bool change = false;
		if (AssetPicker::ResourceDropdown("Base Material", ResourceTypes::GetHash<MaterialData>(), &baseMaterialID, false))
		{
			change = true;
			baseMaterial.Set(uint64_t(baseMaterialID));
		}

		if (!baseMaterialID || !EditorAssetDatabase::AssetExists(baseMaterialID))
		{
			DrawErrorWindow("No base material selected");
			return false;
		}

		EditableResource* pBaseMaterialResource = EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(baseMaterialID);
		if (!pBaseMaterialResource)
		{
			DrawErrorWindow("Invalid base material selected");
			return false;
		}

		YAMLResource<MaterialData>* pBaseMaterial = static_cast<YAMLResource<MaterialData>*>(pBaseMaterialResource);

		Utils::YAMLFileRef& baseFile = **pBaseMaterial;
		auto shaders = baseFile["Shaders"];
		auto baseProperties = baseFile["Properties"];
		auto properties = file["Overrides"];

		static const uint32_t f = ResourceTypes::GetHash<float>();
		static const uint32_t f2 = ResourceTypes::GetHash<glm::vec2>();
		static const uint32_t f3 = ResourceTypes::GetHash<glm::vec3>();
		static const uint32_t f4 = ResourceTypes::GetHash<glm::vec4>();
		static const uint32_t imageDataHash = ResourceTypes::GetHash<ImageData>();

		for (size_t i = 0; i < shaders.Size(); ++i)
		{
			const UUID shaderID = shaders[i]["UUID"].As<uint64_t>();

			EditorShaderData* pEditorShader = EditorShaderProcessor::GetEditorShader(shaderID);
			for (size_t j = 0; j < pEditorShader->m_SamplerNames.size(); ++j)
			{
				const std::string& sampler = pEditorShader->m_SamplerNames[j];
				auto prop = properties[sampler];
				bool enable = prop.Exists() && (prop["Enable"].Exists() && prop["Enable"].As<bool>());

				if (ImGui::Checkbox(("##" + sampler).data(), &enable))
				{
					if (enable)
						prop["Enable"].Set(true);
					else
						prop["Enable"].Set(false);
				}
				ImGui::BeginDisabled(!enable);
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

				if (!prop.Exists()) {
					prop["DisplayName"].Set(sampler);
					prop["ShaderName"].Set(sampler);
					prop["TypeHash"].Set(imageDataHash);
					prop["Value"].Set(0);
				}
				PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(ST_Asset);
				auto baseValue = baseProperties[sampler]["Value"];

				UUID value = enable ? prop["Value"].As<uint64_t>() : baseValue.Exists() ? baseValue.As<uint64_t>() : 0;
				if (AssetPicker::ResourceDropdown(sampler, imageDataHash, &value))
				{
					change = true;
					prop["Value"].Set(value);
				}

				ImGui::EndDisabled();
			}

			for (size_t j = 0; j < pEditorShader->m_PropertyInfos.size(); ++j)
			{
				EditorShaderData::PropertyInfo& info = pEditorShader->m_PropertyInfos[j];
				auto prop = properties[info.m_Name];
				bool enable = prop.Exists() && (prop["Enable"].Exists() && prop["Enable"].As<bool>());

				if (ImGui::Checkbox(info.m_Name.data(), &enable))
				{
					if (enable)
						prop["Enable"].Set(true);
					else
						prop["Enable"].Set(false);
				}
				ImGui::BeginDisabled(!enable);
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

				auto propValue = enable ? prop["Value"] : baseProperties[info.m_Name]["Value"];
				if (info.m_TypeHash == f)
				{
					if (!propValue.Exists())
						propValue.Set(0.0f);
					change |= EditorUI::InputFloat(enable ? file : baseFile, propValue.Path());
				}
				else if (info.m_TypeHash == f2)
				{
					if (!propValue.Exists())
						propValue.Set(glm::vec2{ 0,0 });
					change |= EditorUI::InputFloat2(enable ? file : baseFile, propValue.Path());
				}
				else if (info.m_TypeHash == f3)
				{
					if (!propValue.Exists())
						propValue.Set(glm::vec3{ 0,0,0 });
					change |= EditorUI::InputFloat3(enable ? file : baseFile, propValue.Path());
				}
				else if (info.m_TypeHash == f4)
				{
					if (!propValue.Exists())
						propValue.Set(glm::vec4{ 1,1,1,1 });
					change |= EditorUI::InputColor(enable ? file : baseFile, propValue.Path(), false);
				}

				ImGui::EndDisabled();
			}
		}

		if (change)
		{
			EditorAssetDatabase::SetAssetDirty(pMaterial);
			pMaterial->SetDirty(true);
		}

		const char* error = MaterialEditor::GetMaterialError(pBaseMaterial);
		DrawErrorWindow(error);

		return change;
	}

	void MaterialInstanceEditor::Initialize()
	{
		//MaterialInstanceData* pMaterial = (MaterialInstanceData*)m_pTarget;
		//if (!pMaterial) return;
		//pMaterial->ReloadProperties();
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
