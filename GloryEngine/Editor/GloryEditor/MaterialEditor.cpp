#include "MaterialEditor.h"
#include "PropertyDrawer.h"
#include "AssetPicker.h"
#include "Selection.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"
#include "EditorShaderData.h"
#include "EditorMaterialManager.h"
#include "EditorPipelineManager.h"
#include "EditorResourceManager.h"

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
#include <FileBrowser.h>

namespace Glory::Editor
{
	MaterialEditor::MaterialEditor() {}

	MaterialEditor::~MaterialEditor() {}

	static constexpr float TumbnailSize = 128.0f;

	bool MaterialEditor::OnGUI()
	{
		YAMLResource<MaterialData>* pMaterial = (YAMLResource<MaterialData>*)m_pTarget;
		MaterialData* pMaterialData = EditorApplication::GetInstance()->GetMaterialManager().GetMaterial(pMaterial->GetUUID());

		if (pMaterial->IsSectionedResource() && !pMaterialData)
		{
			ImGui::TextUnformatted("Waiting for material to get imported...");
			return false;
		}

		Utils::NodeValueRef node = **pMaterial;
		auto pipeline = node["Pipeline"];
		UUID pipelineID = pipeline.As<uint64_t>(pMaterialData ? pMaterialData->GetPipelineID() : 0);

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
			change |= PropertiesGUI(pMaterial, pMaterialData);

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

	bool DrawMaterialProperty(Utils::NodeValueRef properties, Utils::NodeValueRef prop, size_t propIndex,
		const MaterialPropertyInfo* propInfo, Serializers& serializers, MaterialData* pMaterialData, Utils::YAMLFileRef& file)
	{
		Utils::NodeValueRef propValue = prop["Value"];
		if (!pMaterialData)
		{
			if (!properties.Exists() || !properties.IsMap())
				properties.SetMap();
			if (!prop.Exists())
			{
				prop["ShaderName"].Set(propInfo->ShaderName());
				prop["TypeHash"].Set(propInfo->TypeHash());
			}
		}

		if (!properties.Exists() || !prop.Exists())
		{
			const bool propertyChange = PropertyDrawer::DrawProperty(propInfo->DisplayName(),
				pMaterialData->Address(propIndex), propInfo->TypeHash(), propInfo->Flags() | PropertyFlags::Color);
			if (propertyChange)
			{
				if (!properties.Exists() || !properties.IsMap())
					properties.SetMap();

				prop["ShaderName"].Set(propInfo->ShaderName());
				prop["TypeHash"].Set(propInfo->TypeHash());

				/* Deserialize value into YAML */
				serializers.SerializeProperty(pMaterialData->GetBufferReference(),
					propInfo->TypeHash(), propInfo->Offset(), propInfo->Size(), propValue);
			}
			return propertyChange;
		}

		const bool propertyChange = PropertyDrawer::DrawProperty(file, propValue.Path(), propInfo->TypeHash(),
			propInfo->TypeHash(), propInfo->Flags() | PropertyFlags::Color);
		if (propertyChange)
		{
			/* Deserialize new value into buffer if the material is loaded */
			if (pMaterialData)
			{
				serializers.DeserializeProperty(pMaterialData->GetBufferReference(),
					propInfo->TypeHash(), propInfo->Offset(), propInfo->Size(), propValue);
			}
		}
		return propertyChange;
	}

	bool DrawResourceProperty(Utils::NodeValueRef properties, Utils::NodeValueRef prop, MaterialData* pMaterialData, const std::string& name,
		Utils::YAMLFileRef& file, EditorRenderImpl* pRenderImpl, std::function<bool(UUID*)> picker, EditorResourceManager& resourceManager)
	{
		static const uint32_t textureDataHash = ResourceTypes::GetHash<TextureData>();

		Utils::NodeValueRef propValue = prop["Value"];

		UUID texID = 0;
		if (pMaterialData)
			pMaterialData->GetTexture(name, &texID);
		const UUID oldValue = propValue.As<uint64_t>(texID);
		UUID value = oldValue;
		const bool textureChange = picker(&value);
		TextureHandle tumbnail = Tumbnail::GetTumbnail(value);
		if (tumbnail)
		{
			constexpr float namePadding = 34.0f;
			ImGui::Image(pRenderImpl->GetTextureID(tumbnail), { TumbnailSize, TumbnailSize });
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right))
			{
				EditableResource* pResource = resourceManager.GetEditableResource(value);
				if (pResource)
					Selection::SetActiveObject(pResource);
			}
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				AssetLocation location;
				if (EditorAssetDatabase::GetAssetLocation(value, location))
				{
					/* Navigate to it in the filebrowser */
					FileBrowser::NavigateToAndHighlight(location.Path);
				}
			}

			const std::string name = EditorAssetDatabase::GetAssetName(value);
			ImGui::SameLine(TumbnailSize + namePadding);
			ImGui::TextUnformatted(name.c_str());
		}

		/* Deserialize new value into resources array */
		if (textureChange)
		{
			if (!properties.Exists() || !properties.IsMap())
				properties.SetMap();

			if (!prop.Exists())
			{
				prop["DisplayName"].Set(name);
				prop["TypeHash"].Set(textureDataHash);
				propValue.Set(0);
			}

			Undo::ApplyYAMLEdit(file, propValue.Path(), uint64_t(oldValue), uint64_t(value));
			const UUID newUUID = propValue.As<uint64_t>();
			if (pMaterialData)
				pMaterialData->SetTexture(name, newUUID);
		}
		return textureChange;
	}

	bool MaterialEditor::PropertiesGUI(YAMLResource<MaterialData>* pMaterial, MaterialData* pMaterialData)
	{
		EditorResourceManager& resourceManager = EditorApplication::GetInstance()->GetResourceManager();
		EditorMaterialManager& materialManager = EditorApplication::GetInstance()->GetMaterialManager();
		EditorPipelineManager& pipelineManager = EditorApplication::GetInstance()->GetPipelineManager();
		Serializers& serializers = EditorApplication::GetInstance()->GetEngine()->GetSerializers();
		EditorRenderImpl* pRenderImpl = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();

		bool change = false;

		Utils::YAMLFileRef& file = pMaterial->File();
		Utils::NodeValueRef node = **pMaterial;
		auto pipeline = node["Pipeline"];
		const UUID pipelineID = pipeline.As<uint64_t>(pMaterialData ? pMaterialData->GetPipelineID() : 0);
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

				auto propOne = properties[propInfoOne->ShaderName()];
				const float start = ImGui::GetCursorPosX();
				const float totalWidth = ImGui::GetContentRegionAvail().x;

				ImGui::PushID(propInfoOne->ShaderName().data());
				EditorUI::PushFlag(EditorUI::Flag::HasSmallButton);
				EditorUI::RemoveButtonPadding = 34.0f;
				change |= DrawMaterialProperty(properties, propOne, pair.first, propInfoOne, serializers, pMaterialData, file);
				EditorUI::RemoveButtonPadding = 24.0f;
				EditorUI::PopFlag();
				ImGui::PopID();

				const std::string& sampler = propInfoTwo->ShaderName();
				auto propTwo = properties[sampler];

				auto propValueTwo = propTwo["Value"];
				ImGui::SameLine();
				ImGui::PushID(sampler.data());
				change |= DrawResourceProperty(properties, propTwo, pMaterialData, sampler, file, pRenderImpl, [&sampler, start, totalWidth](UUID* value) {
					return AssetPicker::ResourceTumbnailButton(sampler, 18.0f, start, totalWidth, textureDataHash, value);
				}, resourceManager);
				ImGui::PopID();
				continue;
			}
			
			/* Draw as normal */
			const MaterialPropertyInfo* propInfo = pPipeline->GetPropertyInfoAt(i);

			if (propInfo->IsResource())
			{
				const std::string& sampler = propInfo->ShaderName();
				auto prop = properties[sampler];
				ImGui::PushID(sampler.data());
				change |= DrawResourceProperty(properties, prop, pMaterialData, sampler, file, pRenderImpl, [&sampler](UUID* value) {
					return AssetPicker::ResourceDropdown(sampler, textureDataHash, value);
				}, resourceManager);
				ImGui::PopID();
				continue;
			}

			ImGui::PushID(propInfo->ShaderName().data());
			auto prop = properties[propInfo->ShaderName()];
			change |= DrawMaterialProperty(properties, prop, i, propInfo, serializers, pMaterialData, file);
			ImGui::PopID();
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
