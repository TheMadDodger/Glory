#include "MaterialInstanceEditor.h"
#include "PropertyDrawer.h"
#include "AssetPicker.h"
#include "AssetManager.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"
#include "EditorShaderData.h"
#include "EditorUI.h"
#include "EditorMaterialManager.h"
#include "EditorPipelineManager.h"

#include <imgui.h>
#include <MaterialEditor.h>
#include <PropertySerializer.h>
#include <IconsFontAwesome6.h>
#include <PipelineData.h>

namespace Glory::Editor
{
	MaterialInstanceEditor::MaterialInstanceEditor() {}

	MaterialInstanceEditor::~MaterialInstanceEditor() {}

	bool MaterialInstanceEditor::OnGUI()
	{
		EditorMaterialManager& materialManager = EditorApplication::GetInstance()->GetMaterialManager();
		EditorPipelineManager& pipelineManager = EditorApplication::GetInstance()->GetPipelineManager();
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
			materialManager.SetMaterialInstanceBaseMaterial(pMaterial->GetUUID(), baseMaterialID);
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
		auto pipeline = baseFile["Pipeline"];
		const UUID pipelineID = pipeline.As<uint64_t>();
		if (pipelineID == 0)
			return false;
		PipelineData* pPipeline = pipelineManager.GetPipelineData(pipelineID);
		if (!pPipeline)
		{
			ImGui::TextColored({ 1,0,0,1 }, "The chosen pipeline is not yet compiled");
			return false;
		}

		if (EditorUI::Header("Overrides"))
		{
			auto baseProperties = baseFile["Properties"];
			auto properties = file["Overrides"];

			for (size_t i = 0; i < pPipeline->PropertyInfoCount(); ++i)
			{
				const MaterialPropertyInfo* propInfo = pPipeline->GetPropertyInfoAt(i);

				size_t materialPropertyIndex = 0;
				if (!pMaterialData->GetPropertyInfoIndex(materialManager, propInfo->ShaderName(), materialPropertyIndex))
					continue;
				MaterialPropertyInfo* pMaterialProperty = pMaterialData->GetPropertyInfoAt(materialManager, materialPropertyIndex);

				if (propInfo->IsResource())
				{
					const std::string& sampler = propInfo->ShaderName();
					auto prop = properties[sampler];
					bool enable = prop.Exists() && (prop["Enable"].Exists() && prop["Enable"].As<bool>());

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

							if (!prop["Value"].Exists() && baseProperties[sampler]["Value"].Exists())
								prop["Value"].Set(baseProperties[sampler]["Value"].Node());
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

					if (!prop.Exists() && baseProperties[sampler]["Value"].Exists())
					{
						prop["Value"].Set(baseProperties[sampler]["Value"].Node());
						prop["Enable"].Set(enable);
					}
					auto baseValue = baseProperties[sampler]["Value"];

					auto propValue = prop["Value"];

					PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(ST_Asset);

					Undo::StartRecord("Material Instance Property");
					ImGui::BeginDisabled(!enable);
					change |= pPropertyDrawer->Draw(enable ? file : baseFile, enable ? propValue.Path() : baseValue.Path(), pMaterialProperty->TypeHash(), pMaterialProperty->Flags());
					ImGui::EndDisabled();
					ImGui::PopID();
					Undo::StopRecord();

					if (!enable) continue;
					/* Deserialize new value into resources array */
					const UUID newUUID = propValue.As<uint64_t>(0);
					pMaterialData->SetTexture(materialManager, sampler, newUUID);
					continue;
				}

				auto prop = properties[propInfo->ShaderName()];
				if (!prop["Enable"].Exists())
					prop["Enable"].Set(false);

				bool enable = prop.Exists() && (prop["Enable"].Exists() && prop["Enable"].As<bool>());

				ImGui::PushID(propInfo->ShaderName().data());
				EditorUI::PushFlag(EditorUI::Flag::NoLabel);
				if (EditorUI::CheckBox(file, prop["Enable"].Path()))
				{
					enable = prop["Enable"].As<bool>();
					if (enable)
					{
						prop["Enable"].Set(true);
						pMaterialData->EnableProperty(materialPropertyIndex);

						if (!prop["Value"].Exists() && baseProperties[propInfo->ShaderName()]["Value"].Exists())
							prop["Value"].Set(baseProperties[propInfo->ShaderName()]["Value"].Node());
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

				auto baseValue = baseProperties[propInfo->ShaderName()]["Value"];
				auto propValue = prop["Value"];

				Undo::StartRecord("Material Instance Property");
				ImGui::BeginDisabled(!enable);
				change |= PropertyDrawer::DrawProperty(enable ? file : baseFile, enable ? propValue.Path() : baseValue.Path(), propInfo->TypeHash(), propInfo->TypeHash(), pMaterialProperty->Flags());
				ImGui::EndDisabled();
				ImGui::PopID();
				Undo::StopRecord();

				if (!enable) continue;
				/* Deserialize new value into buffer */
				serializers.DeserializeProperty(pMaterialData->GetBufferReference(materialManager),
					pMaterialProperty->TypeHash(), pMaterialProperty->Offset(), pMaterialProperty->Size(), propValue);
			}
		}

		if (change)
		{
			EditorAssetDatabase::SetAssetDirty(pMaterial);
			pMaterial->SetDirty(true);
		}

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
