#include "MaterialInstanceEditor.h"
#include "PropertyDrawer.h"
#include "AssetPicker.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"
#include "EditorShaderProcessor.h"
#include "EditorShaderData.h"
#include "EditorUI.h"
#include "EditorMaterialManager.h"

#include <imgui.h>
#include <MaterialEditor.h>
#include <PropertySerializer.h>
#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	MaterialInstanceEditor::MaterialInstanceEditor() {}

	MaterialInstanceEditor::~MaterialInstanceEditor() {}

	bool MaterialInstanceEditor::OnGUI()
	{
		EditorMaterialManager& materialManager = EditorApplication::GetInstance()->GetMaterialManager();
		Serializers& serializers = EditorApplication::GetInstance()->GetEngine()->GetSerializers();
		YAMLResource<MaterialInstanceData>* pMaterial = (YAMLResource<MaterialInstanceData>*)m_pTarget;
		MaterialInstanceData* pMaterialData = EditorApplication::GetInstance()->GetMaterialManager().GetMaterialInstance(pMaterial->GetUUID());

		Utils::YAMLFileRef& file = **pMaterial;
		auto baseMaterial = file["BaseMaterial"];
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

		for (size_t i = 0; i < shaders.Size(); ++i)
		{
			const UUID shaderID = shaders[i]["UUID"].As<uint64_t>();

			EditorShaderData* pEditorShader = EditorShaderProcessor::GetEditorShader(shaderID);
			for (size_t j = 0; j < pEditorShader->m_SamplerNames.size(); ++j)
			{
				const std::string& sampler = pEditorShader->m_SamplerNames[j];
				auto prop = properties[sampler];
				bool enable = prop.Exists() && (prop["Enable"].Exists() && prop["Enable"].As<bool>());

				size_t materialPropertyIndex = 0;
				if (!pMaterialData->GetPropertyInfoIndex(materialManager, sampler, materialPropertyIndex))
					continue;

				if (!prop["Enable"].Exists())
					prop["Enable"].Set(false);

				ImGui::PushID(sampler.data());
				EditorUI::PushFlag(EditorUI::Flag::NoLabel);
				if (EditorUI::CheckBox(file, prop["Enable"].Path()))
				{
					enable = prop["Enable"].As<bool>();
					if (enable)
					{
						prop["Enable"].Set(true);
						pMaterialData->EnableProperty(materialPropertyIndex);
					}
					else
					{
						prop["Enable"].Set(false);
						pMaterialData->DisableProperty(materialPropertyIndex);
					}
					change = true;
				}
				EditorUI::PopFlag();
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

				if (!prop.Exists()) {
					prop["Value"].Set(0);
					prop["Enable"].Set(enable);
				}
				auto baseValue = baseProperties[sampler]["Value"];

				auto propValue = prop["Value"];

				MaterialPropertyInfo* pMaterialProperty = pMaterialData->GetPropertyInfoAt(materialManager, materialPropertyIndex);
				PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(ST_Asset);

				Undo::StartRecord("Material Instance Property");
				ImGui::BeginDisabled(!enable);
				change |= pPropertyDrawer->Draw(enable ? file : baseFile, enable ? propValue.Path() : baseValue.Path(), pMaterialProperty->TypeHash(), pMaterialProperty->Flags());
				ImGui::EndDisabled();
				ImGui::PopID();
				Undo::StopRecord();

				if (!enable) continue;
				/* Deserialize new value into resources array */
				const UUID newUUID = propValue.As<uint64_t>();
				pMaterialData->SetTexture(materialManager, sampler, newUUID);
			}

			for (size_t j = 0; j < pEditorShader->m_PropertyInfos.size(); ++j)
			{
				EditorShaderData::PropertyInfo& info = pEditorShader->m_PropertyInfos[j];
				size_t materialPropertyIndex = 0;
				if (!pMaterialData->GetPropertyInfoIndex(materialManager, info.m_Name, materialPropertyIndex))
					continue;
				MaterialPropertyInfo* pMaterialProperty = pMaterialData->GetPropertyInfoAt(materialManager, materialPropertyIndex);

				auto prop = properties[info.m_Name];
				if (!prop["Enable"].Exists())
					prop["Enable"].Set(false);

				bool enable = prop.Exists() && (prop["Enable"].Exists() && prop["Enable"].As<bool>());

				ImGui::PushID(info.m_Name.data());
				EditorUI::PushFlag(EditorUI::Flag::NoLabel);
				if (EditorUI::CheckBox(file, prop["Enable"].Path()))
				{
					enable = prop["Enable"].As<bool>();
					if (enable)
					{
						prop["Enable"].Set(true);
						pMaterialData->EnableProperty(materialPropertyIndex);
					}
					else
					{
						prop["Enable"].Set(false);
						pMaterialData->DisableProperty(materialPropertyIndex);
					}
					change = true;
				}
				EditorUI::PopFlag();
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

				auto baseValue = baseProperties[info.m_Name]["Value"];
				auto propValue = prop["Value"];

				Undo::StartRecord("Material Instance Property");
				ImGui::BeginDisabled(!enable);
				change |= PropertyDrawer::DrawProperty(enable ? file : baseFile, enable ? propValue.Path() : baseValue.Path(), info.m_TypeHash, info.m_TypeHash, pMaterialProperty->Flags());
				ImGui::EndDisabled();
				ImGui::PopID();
				Undo::StopRecord();

				if (!enable) continue;
				/* Deserialize new value into buffer */
				serializers.DeserializeProperty(pMaterialData->GetBufferReference(materialManager),
					pMaterialProperty->TypeHash(), pMaterialProperty->Offset(), pMaterialProperty->Size(), propValue.Node());
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
