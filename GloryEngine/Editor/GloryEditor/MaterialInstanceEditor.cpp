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
#include "Tumbnail.h"

#include <imgui.h>
#include <MaterialEditor.h>
#include <PropertySerializer.h>
#include <IconsFontAwesome6.h>
#include <PipelineData.h>

namespace Glory::Editor
{
	static constexpr float TumbnailSize = 128.0f;

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
			Undo::StartRecord("Material Instance Property", pMaterialData->GetUUID(), true);
			change |= PropertiesGUI(file, baseFile, pMaterialData, pPipeline);
			Undo::StopRecord();
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

	bool MaterialInstanceEditor::PropertiesGUI(Utils::YAMLFileRef& file, Utils::YAMLFileRef& baseFile, MaterialInstanceData* pMaterialData, PipelineData* pPipeline)
	{
		bool change = false;

		EditorMaterialManager& materialManager = EditorApplication::GetInstance()->GetMaterialManager();
		EditorPipelineManager& pipelineManager = EditorApplication::GetInstance()->GetPipelineManager();
		Serializers& serializers = EditorApplication::GetInstance()->GetEngine()->GetSerializers();
		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();

		/* Group properties */
		std::vector<std::pair<size_t, size_t>> propertyPairs;
		MaterialEditor::GeneratePropertyPairs(pPipeline, propertyPairs);
		static const uint32_t textureDataHash = ResourceTypes::GetHash<TextureData>();

		auto baseProperties = baseFile["Properties"];
		auto properties = file["Overrides"];
		for (size_t i = 0; i < pPipeline->PropertyInfoCount(); ++i)
		{
			auto pairIter = std::find_if(propertyPairs.begin(), propertyPairs.end(), [i](std::pair<size_t, size_t>& pair) { return pair.first == i; });
			if (pairIter != propertyPairs.end()) continue;
			pairIter = std::find_if(propertyPairs.begin(), propertyPairs.end(), [i](std::pair<size_t, size_t>& pair) { return pair.second == i; });
			if (pairIter != propertyPairs.end())
			{
				/* Draw the pair instead */
				std::pair<size_t, size_t>& pair = *pairIter;
				const MaterialPropertyInfo* propInfoOne = pPipeline->GetPropertyInfoAt(pair.first);
				const MaterialPropertyInfo* propInfoTwo = pPipeline->GetPropertyInfoAt(pair.second);

				size_t materialPropertyIndexOne = 0;
				size_t materialPropertyIndexTwo = 0;
				if (!pMaterialData->GetPropertyInfoIndex(materialManager, propInfoOne->ShaderName(), materialPropertyIndexOne))
					continue;
				if (!pMaterialData->GetPropertyInfoIndex(materialManager, propInfoTwo->ShaderName(), materialPropertyIndexTwo))
					continue;

				MaterialPropertyInfo* pMaterialPropertyOne = pMaterialData->GetPropertyInfoAt(materialManager, materialPropertyIndexOne);
				MaterialPropertyInfo* pMaterialPropertyTwo = pMaterialData->GetPropertyInfoAt(materialManager, materialPropertyIndexTwo);

				auto propOne = properties[propInfoOne->ShaderName()];
				if (!propOne["Enable"].Exists())
					propOne["Enable"].Set(false);

				bool enableOne = propOne.Exists() && (propOne["Enable"].Exists() && propOne["Enable"].As<bool>());
				const std::string& sampler = propInfoTwo->ShaderName();
				auto propTwo = properties[sampler];
				bool enableTwo = propTwo.Exists() && (propTwo["Enable"].Exists() && propTwo["Enable"].As<bool>());
				bool enable = enableOne || enableTwo;
				propOne["Enable"].Set(enable);
				propTwo["Enable"].Set(enable);

				if (enable)
				{
					if (!enableOne) pMaterialData->EnableProperty(materialPropertyIndexOne);
					if (!enableTwo) pMaterialData->EnableProperty(materialPropertyIndexTwo);
				}

				const float start = ImGui::GetCursorPosX();
				const float totalWidth = ImGui::GetContentRegionAvail().x;
				ImGui::PushID(i);
				EditorUI::PushFlag(EditorUI::Flag::NoLabel);
				if (EditorUI::CheckBox("enable", &enable))
				{
					if (enable)
					{
						Undo::ApplyYAMLEdit(file, propOne["Enable"].Path(), false, true);
						Undo::ApplyYAMLEdit(file, propTwo["Enable"].Path(), false, true);
						if (!propOne["Value"].Exists() && baseProperties[propInfoOne->ShaderName()]["Value"].Exists())
							propOne["Value"].Set(baseProperties[propInfoOne->ShaderName()]["Value"].Node());
						if (!propTwo.Exists() && baseProperties[sampler]["Value"].Exists())
							propTwo["Value"].Set(baseProperties[sampler]["Value"].Node());
						pMaterialData->EnableProperty(materialPropertyIndexOne);
					}
					else
					{
						Undo::ApplyYAMLEdit(file, propOne["Enable"].Path(), true, false);
						Undo::ApplyYAMLEdit(file, propTwo["Enable"].Path(), true, false);
						pMaterialData->DisableProperty(materialPropertyIndexTwo);
					}
					change = true;
				}
				EditorUI::PopFlag();
				ImGui::PopID();

				ImGui::BeginDisabled(!enable);

				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

				ImGui::PushID(pMaterialPropertyOne->ShaderName().data());
				auto baseValueOne = baseProperties[propInfoOne->ShaderName()]["Value"];
				auto propValueOne = propOne["Value"];
				EditorUI::PushFlag(EditorUI::Flag::HasSmallButton);
				EditorUI::RemoveButtonPadding = 34.0f;
				change |= PropertyDrawer::DrawProperty(enable ? file : baseFile, enable ? propValueOne.Path() : baseValueOne.Path(), propInfoOne->TypeHash(), propInfoOne->TypeHash(), pMaterialPropertyOne->Flags());
				EditorUI::RemoveButtonPadding = 24.0f;
				EditorUI::PopFlag();
				ImGui::PopID();

				auto baseValueTwo = baseProperties[sampler]["Value"];
				auto propValueTwo = propTwo["Value"];

				const UUID oldValue = enable ? propValueTwo.As<uint64_t>(0) : baseValueTwo.As<uint64_t>(0);
				UUID value = oldValue;
				ImGui::SameLine();
				ImGui::PushID(sampler.data());
				const bool textureChange = AssetPicker::ResourceTumbnailButton("value", 18.0f, start, totalWidth, textureDataHash, &value);
				Texture* pTumbnail = Tumbnail::GetTumbnail(value);
				if (pTumbnail)
					ImGui::Image(pRenderImpl->GetTextureID(pTumbnail), { TumbnailSize, TumbnailSize });
				ImGui::PopID();
				ImGui::EndDisabled();

				if (textureChange)
					Undo::ApplyYAMLEdit(file, propValueTwo.Path(), uint64_t(oldValue), uint64_t(value));

				if (enable)
				{
					serializers.DeserializeProperty(pMaterialData->GetBufferReference(materialManager),
						pMaterialPropertyOne->TypeHash(), pMaterialPropertyOne->Offset(), pMaterialPropertyOne->Size(), propValueOne);
					pMaterialData->SetTexture(materialManager, sampler, value);
				}

				continue;
			}

			/* Draw as normal */
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

				ImGui::BeginDisabled(!enable);
				const UUID value = enable ? propValue.As<uint64_t>(0) : baseValue.As<uint64_t>(0);
				change |= pPropertyDrawer->Draw(enable ? file : baseFile, enable ? propValue.Path() : baseValue.Path(), pMaterialProperty->TypeHash(), pMaterialProperty->Flags());
				Texture* pTumbnail = Tumbnail::GetTumbnail(value);
				if (pTumbnail)
					ImGui::Image(pRenderImpl->GetTextureID(pTumbnail), { TumbnailSize, TumbnailSize });
				ImGui::EndDisabled();
				ImGui::PopID();

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

			ImGui::BeginDisabled(!enable);
			change |= PropertyDrawer::DrawProperty(enable ? file : baseFile, enable ? propValue.Path() : baseValue.Path(), propInfo->TypeHash(), propInfo->TypeHash(), pMaterialProperty->Flags());
			ImGui::EndDisabled();
			ImGui::PopID();

			if (!enable) continue;
			/* Deserialize new value into buffer */
			serializers.DeserializeProperty(pMaterialData->GetBufferReference(materialManager),
				pMaterialProperty->TypeHash(), pMaterialProperty->Offset(), pMaterialProperty->Size(), propValue);
		}

		return change;
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
