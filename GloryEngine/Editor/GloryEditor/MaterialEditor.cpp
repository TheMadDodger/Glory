#include "MaterialEditor.h"
#include "PropertyDrawer.h"
#include "AssetPicker.h"
#include "Selection.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"
#include "EditorShaderData.h"
#include "EditorMaterialManager.h"
#include "EditorPipelineManager.h"

#include <imgui.h>
#include <ResourceType.h>
#include <PropertySerializer.h>
#include <ShaderSourceData.h>
#include <GLORY_YAML.h>
#include <AssetDatabase.h>
#include <AssetManager.h>
#include <EditorUI.h>
#include <PipelineData.h>

#include <IconsFontAwesome6.h>
#include <PropertyFlags.h>
#include <Tumbnail.h>

namespace Glory::Editor
{
	MaterialEditor::MaterialEditor() {}

	MaterialEditor::~MaterialEditor() {}

	static constexpr float TumbnailSize = 128.0f;

	bool MaterialEditor::OnGUI()
	{
		YAMLResource<MaterialData>* pMaterial = (YAMLResource<MaterialData>*)m_pTarget;
		MaterialData* pMaterialData = EditorApplication::GetInstance()->GetMaterialManager().GetMaterial(pMaterial->GetUUID());

		Utils::NodeValueRef node = **pMaterial;
		auto pipeline = node["Pipeline"];
		UUID pipelineID = pipeline.Exists() ? pipeline.As<uint64_t>() : 0;

		bool change = false;
		if (AssetPicker::ResourceDropdown("Pipeline", ResourceTypes::GetHash<PipelineData>(), &pipelineID))
		{
			EditorApplication::GetInstance()->GetMaterialManager().SetMaterialPipeline(pMaterial->GetUUID(), pipelineID);
			change = true;
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		if (EditorUI::Header("Properties"))
		{
			change |= PropertiesGUI(pMaterial, pMaterialData);
		}

		if (change)
		{
			EditorAssetDatabase::SetAssetDirty(pMaterial);
			pMaterial->SetDirty(true);
			pMaterialData->SetDirty(true);
		}
		return change;
	}

	void MaterialEditor::GeneratePropertyPairs(PipelineData* pPipeline, std::vector<std::pair<size_t, size_t>>& propertyPairs)
	{
		for (size_t i = 0; i < pPipeline->PropertyInfoCount(); ++i)
		{
			const MaterialPropertyInfo* propInfo = pPipeline->GetPropertyInfoAt(i);
			if (!propInfo->IsResource()) continue;
			std::string_view name = propInfo->ShaderName();
			const size_t samplerNameEnd = name.find("Sampler");
			if (samplerNameEnd != std::string_view::npos)
				name = name.substr(0, samplerNameEnd);
			if (name == "tex") name = "color";
			if (name == "metalness") name = "metallic";
			name = name.substr(1);

			/* Find a non-resource property that matches this resource property */
			bool found = false;
			for (size_t j = 0; j < pPipeline->PropertyInfoCount(); ++j)
			{
				const MaterialPropertyInfo* otherProp = pPipeline->GetPropertyInfoAt(j);
				if (otherProp->IsResource()) continue;
				std::string_view otherName = otherProp->ShaderName();
				otherName = otherName.substr(1);
				if (name.find(otherName) == std::string::npos && otherName.find(name) == std::string::npos) continue;
				propertyPairs.push_back({ j, i });
				break;
			}
		}
	}

	bool MaterialEditor::PropertiesGUI(YAMLResource<MaterialData>* pMaterial, MaterialData* pMaterialData)
	{
		EditorMaterialManager& materialManager = EditorApplication::GetInstance()->GetMaterialManager();
		EditorPipelineManager& pipelineManager = EditorApplication::GetInstance()->GetPipelineManager();
		Serializers& serializers = EditorApplication::GetInstance()->GetEngine()->GetSerializers();
		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();

		bool change = false;

		Utils::YAMLFileRef& file = pMaterial->File();
		Utils::NodeValueRef node = **pMaterial;
		auto pipeline = node["Pipeline"];
		const UUID pipelineID = pipeline.As<uint64_t>();
		if (pipelineID == 0)
			return false;
		PipelineData* pPipeline = pipelineManager.GetPipelineData(pipelineID);
		if (!pPipeline)
		{
			ImGui::TextColored({ 1,0,0,1 }, "The chosen pipeline is not yet compiled");
			return false;
		}

		/* Group properties */
		std::vector<std::pair<size_t, size_t>> propertyPairs;
		GeneratePropertyPairs(pPipeline, propertyPairs);

		auto properties = node["Properties"];
		static const uint32_t textureDataHash = ResourceTypes::GetHash<TextureData>();

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
				if (!pMaterialData->GetPropertyInfoIndex(propInfoOne->ShaderName(), materialPropertyIndexOne))
					continue;
				if (!pMaterialData->GetPropertyInfoIndex(propInfoTwo->ShaderName(), materialPropertyIndexTwo))
					continue;

				MaterialPropertyInfo* pMaterialPropertyOne = pMaterialData->GetPropertyInfoAt(materialPropertyIndexOne);
				MaterialPropertyInfo* pMaterialPropertyTwo = pMaterialData->GetPropertyInfoAt(materialPropertyIndexTwo);

				auto propOne = properties[propInfoOne->ShaderName()];
				if (!propOne.Exists()) {
					propOne["ShaderName"].Set(propInfoOne->ShaderName());
					propOne["TypeHash"].Set(propInfoOne->TypeHash());
				}

				const float start = ImGui::GetCursorPosX();
				const float totalWidth = ImGui::GetContentRegionAvail().x;

				ImGui::PushID(pMaterialPropertyOne->ShaderName().data());
				auto propValueOne = propOne["Value"];
				EditorUI::PushFlag(EditorUI::Flag::HasSmallButton);
				EditorUI::RemoveButtonPadding = 34.0f;
				const bool propertyChange = PropertyDrawer::DrawProperty(file, propValueOne.Path(), propInfoOne->TypeHash(), propInfoOne->TypeHash(), pMaterialPropertyOne->Flags() | PropertyFlags::Color);
				EditorUI::RemoveButtonPadding = 24.0f;
				EditorUI::PopFlag();
				ImGui::PopID();

				/* Deserialize new value into buffer */
				if (propertyChange)
				{
					serializers.DeserializeProperty(pMaterialData->GetBufferReference(),
						pMaterialPropertyOne->TypeHash(), pMaterialPropertyOne->Offset(), pMaterialPropertyOne->Size(), propValueOne);
					change = true;
				}

				const std::string& sampler = propInfoTwo->ShaderName();

				auto propTwo = properties[sampler];
				if (!propTwo.Exists()) {
					propTwo["DisplayName"].Set(sampler);
					propTwo["TypeHash"].Set(textureDataHash);
					propTwo["Value"].Set(0);
				}

				auto propValueTwo = propTwo["Value"];
				ImGui::SameLine();
				ImGui::PushID(sampler.data());
				const UUID oldValue = propTwo["Value"].As<uint64_t>();
				UUID value = oldValue;
				const bool textureChange = AssetPicker::ResourceTumbnailButton("value", 18.0f, start, totalWidth, textureDataHash, &value);
				TextureHandle tumbnail = Tumbnail::GetTumbnail(value);
				if (tumbnail)
					ImGui::Image(pRenderImpl->GetTextureID(tumbnail), { TumbnailSize, TumbnailSize });
				ImGui::PopID();

				/* Deserialize new value into resources array */
				if (textureChange)
				{
					Undo::ApplyYAMLEdit(file, propTwo["Value"].Path(), uint64_t(oldValue), uint64_t(value));
					const UUID newUUID = propValueTwo.As<uint64_t>();
					pMaterialData->SetTexture(sampler, newUUID);
					change = true;
				}
				continue;
			}
			
			/* Draw as normal */
			const MaterialPropertyInfo* propInfo = pPipeline->GetPropertyInfoAt(i);

			size_t materialPropertyIndex = 0;
			if (!pMaterialData->GetPropertyInfoIndex(propInfo->ShaderName(), materialPropertyIndex))
				continue;

			MaterialPropertyInfo* pMaterialProperty = pMaterialData->GetPropertyInfoAt(materialPropertyIndex);

			if (propInfo->IsResource())
			{
				const std::string& sampler = propInfo->ShaderName();

				auto prop = properties[sampler];
				if (!prop.Exists()) {
					prop["DisplayName"].Set(sampler);
					prop["TypeHash"].Set(textureDataHash);
					prop["Value"].Set(0);
				}

				auto propValue = prop["Value"];
				PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(ST_Asset);

				ImGui::PushID(sampler.data());
				const bool changed = pPropertyDrawer->Draw(file, propValue.Path(), pMaterialProperty->TypeHash(), pMaterialProperty->Flags());
				TextureHandle tumbnail = Tumbnail::GetTumbnail(propValue.As<uint64_t>());
				if (tumbnail)
					ImGui::Image(pRenderImpl->GetTextureID(tumbnail), { TumbnailSize, TumbnailSize });
				ImGui::PopID();

				/* Deserialize new value into resources array */
				if (changed)
				{
					const UUID newUUID = propValue.As<uint64_t>();
					pMaterialData->SetTexture(sampler, newUUID);
					change = true;
				}
				continue;
			}

			auto prop = properties[propInfo->ShaderName()];
			if (!prop.Exists()) {
				prop["ShaderName"].Set(propInfo->ShaderName());
				prop["TypeHash"].Set(propInfo->TypeHash());
			}

			ImGui::PushID(propInfo->ShaderName().data());
			auto propValue = prop["Value"];
			const bool changed = PropertyDrawer::DrawProperty(file, propValue.Path(), propInfo->TypeHash(), propInfo->TypeHash(), pMaterialProperty->Flags() | PropertyFlags::Color);
			ImGui::PopID();

			/* Deserialize new value into buffer */
			if (changed)
			{
				serializers.DeserializeProperty(pMaterialData->GetBufferReference(),
					pMaterialProperty->TypeHash(), pMaterialProperty->Offset(), pMaterialProperty->Size(), propValue);
				change = true;
			}
		}
		return change;
	}

	StaticMaterialEditor::StaticMaterialEditor()
	{
	}

	StaticMaterialEditor::~StaticMaterialEditor()
	{
	}

	bool StaticMaterialEditor::OnGUI()
	{
		EditorMaterialManager& materialManager = EditorApplication::GetInstance()->GetMaterialManager();
		MaterialData* pMaterialData = static_cast<MaterialData*>(m_pTarget);

		UUID pipelineID = pMaterialData->GetPipelineID();

		ImGui::BeginDisabled(true);
		AssetPicker::ResourceDropdown("Pipeline", ResourceTypes::GetHash<PipelineData>(), &pipelineID);

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		if (EditorUI::Header("Properties"))
		{
			PropertiesGUI(pMaterialData);
		}
		ImGui::EndDisabled();

		return false;
	}

	void StaticMaterialEditor::PropertiesGUI(MaterialData* pMaterialData)
	{
		EditorMaterialManager& materialManager = EditorApplication::GetInstance()->GetMaterialManager();
		EditorPipelineManager& pipelineManager = EditorApplication::GetInstance()->GetPipelineManager();
		Serializers& serializers = EditorApplication::GetInstance()->GetEngine()->GetSerializers();
		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();

		const UUID pipelineID = pMaterialData->GetPipelineID();
		if (pipelineID == 0)
			return;
		PipelineData* pPipeline = pipelineManager.GetPipelineData(pipelineID);
		if (!pPipeline)
		{
			ImGui::TextColored({ 1,0,0,1 }, "The chosen pipeline is not yet compiled");
			return;
		}

		/* Group properties */
		std::vector<std::pair<size_t, size_t>> propertyPairs;
		MaterialEditor::GeneratePropertyPairs(pPipeline, propertyPairs);

		static const uint32_t textureDataHash = ResourceTypes::GetHash<TextureData>();
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
				if (!pMaterialData->GetPropertyInfoIndex(propInfoOne->ShaderName(), materialPropertyIndexOne))
					continue;
				if (!pMaterialData->GetPropertyInfoIndex(propInfoTwo->ShaderName(), materialPropertyIndexTwo))
					continue;

				MaterialPropertyInfo* pMaterialPropertyOne = pMaterialData->GetPropertyInfoAt(materialPropertyIndexOne);
				MaterialPropertyInfo* pMaterialPropertyTwo = pMaterialData->GetPropertyInfoAt(materialPropertyIndexTwo);

				const float start = ImGui::GetCursorPosX();
				const float totalWidth = ImGui::GetContentRegionAvail().x;

				ImGui::PushID(propInfoOne->ShaderName().data());
				void* pAddress = pMaterialData->Address(materialPropertyIndexOne);
				EditorUI::PushFlag(EditorUI::Flag::HasSmallButton);
				EditorUI::RemoveButtonPadding = 34.0f;
				PropertyDrawer::DrawProperty(propInfoOne->DisplayName(), pAddress, propInfoOne->TypeHash(), pMaterialPropertyOne->Flags() | PropertyFlags::Color);
				EditorUI::RemoveButtonPadding = 24.0f;
				EditorUI::PopFlag();
				ImGui::PopID();

				const std::string& sampler = propInfoTwo->ShaderName();
				auto resourceId = pMaterialData->GetResourceUUIDPointer(pMaterialPropertyTwo->Offset());
				ImGui::SameLine();
				ImGui::PushID(sampler.data());
				const bool textureChange = AssetPicker::ResourceTumbnailButton("value", 18.0f, start, totalWidth, textureDataHash, resourceId->AssetUUIDMember());
				TextureHandle tumbnail = Tumbnail::GetTumbnail(resourceId->AssetUUID());
				if (tumbnail)
					ImGui::Image(pRenderImpl->GetTextureID(tumbnail), { TumbnailSize, TumbnailSize });
				ImGui::PopID();

				continue;
			}

			/* Draw as normal */
			const MaterialPropertyInfo* propInfo = pPipeline->GetPropertyInfoAt(i);

			size_t materialPropertyIndex = 0;
			if (!pMaterialData->GetPropertyInfoIndex(propInfo->ShaderName(), materialPropertyIndex))
				continue;

			MaterialPropertyInfo* pMaterialProperty = pMaterialData->GetPropertyInfoAt(materialPropertyIndex);

			if (propInfo->IsResource())
			{
				auto resourceId = pMaterialData->GetResourceUUIDPointer(pMaterialProperty->Offset());
				const std::string& sampler = propInfo->ShaderName();
				PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(ST_Asset);

				ImGui::PushID(sampler.data());
				pPropertyDrawer->Draw(pMaterialProperty->DisplayName(), resourceId, pMaterialProperty->TypeHash(), pMaterialProperty->Flags());
				TextureHandle tumbnail = Tumbnail::GetTumbnail(resourceId->AssetUUID());
				if (tumbnail)
					ImGui::Image(pRenderImpl->GetTextureID(tumbnail), { TumbnailSize, TumbnailSize });
				ImGui::PopID();
				continue;
			}

			ImGui::PushID(propInfo->ShaderName().data());
			void* pAddress = pMaterialData->Address(i);
			PropertyDrawer::DrawProperty(propInfo->DisplayName(), pAddress, propInfo->TypeHash(), pMaterialProperty->Flags() | PropertyFlags::Color);
			ImGui::PopID();
		}
	}
}
