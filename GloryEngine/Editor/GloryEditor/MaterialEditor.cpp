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

namespace Glory::Editor
{
	MaterialEditor::MaterialEditor() {}

	MaterialEditor::~MaterialEditor() {}

	bool MaterialEditor::OnGUI()
	{
		YAMLResource<MaterialData>* pMaterial = (YAMLResource<MaterialData>*)m_pTarget;
		MaterialData* pMaterialData = EditorApplication::GetInstance()->GetMaterialManager().GetMaterial(pMaterial->GetUUID());

		Utils::YAMLFileRef file = **pMaterial;
		auto pipeline = file["Pipeline"];
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
			Undo::StartRecord("Material Property", pMaterial->GetUUID());
			change = PropertiesGUI(pMaterial, pMaterialData);
			Undo::StopRecord();
		}

		if (change)
		{
			EditorAssetDatabase::SetAssetDirty(pMaterial);
			pMaterial->SetDirty(true);
		}
		return change;
	}

	bool MaterialEditor::PropertiesGUI(YAMLResource<MaterialData>* pMaterial, MaterialData* pMaterialData)
	{
		EditorMaterialManager& materialManager = EditorApplication::GetInstance()->GetMaterialManager();
		EditorPipelineManager& pipelineManager = EditorApplication::GetInstance()->GetPipelineManager();
		Serializers& serializers = EditorApplication::GetInstance()->GetEngine()->GetSerializers();

		bool change = false;

		Utils::YAMLFileRef& file = **pMaterial;
		auto pipeline = file["Pipeline"];
		const UUID pipelineID = pipeline.As<uint64_t>();
		if (pipelineID == 0)
			return false;
		PipelineData* pPipeline = pipelineManager.GetPipelineData(pipelineID);
		if (!pPipeline)
		{
			ImGui::TextColored({ 1,0,0,1 }, "The chosen pipeline is not yet compiled");
			return false;
		}

		auto properties = file["Properties"];
		static const uint32_t textureDataHash = ResourceTypes::GetHash<TextureData>();

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
				continue;
			}

			auto prop = properties[propInfo->ShaderName()];
			if (!prop.Exists()) {
				prop["ShaderName"].Set(propInfo->ShaderName());
				prop["TypeHash"].Set(propInfo->TypeHash());
			}

			ImGui::PushID(propInfo->ShaderName().data());
			auto propValue = prop["Value"];
			change |= PropertyDrawer::DrawProperty(file, propValue.Path(), propInfo->TypeHash(), propInfo->TypeHash(), pMaterialProperty->Flags());
			ImGui::PopID();

			/* Deserialize new value into buffer */
			if (change)
			{
				serializers.DeserializeProperty(pMaterialData->GetBufferReference(materialManager),
					pMaterialProperty->TypeHash(), pMaterialProperty->Offset(), pMaterialProperty->Size(), propValue);
			}
		}

		if (change)
		{
			pMaterial->SetDirty(true);
			EditorAssetDatabase::SetAssetDirty(pMaterial);
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

		UUID pipelineID = pMaterialData->GetPipelineID(materialManager);

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

		const UUID pipelineID = pMaterialData->GetPipelineID(materialManager);
		if (pipelineID == 0)
			return;
		PipelineData* pPipeline = pipelineManager.GetPipelineData(pipelineID);
		if (!pPipeline)
		{
			ImGui::TextColored({ 1,0,0,1 }, "The chosen pipeline is not yet compiled");
			return;
		}

		static const uint32_t textureDataHash = ResourceTypes::GetHash<TextureData>();

		for (size_t i = 0; i < pPipeline->PropertyInfoCount(); ++i)
		{
			const MaterialPropertyInfo* propInfo = pPipeline->GetPropertyInfoAt(i);

			size_t materialPropertyIndex = 0;
			if (!pMaterialData->GetPropertyInfoIndex(materialManager, propInfo->ShaderName(), materialPropertyIndex))
				continue;

			MaterialPropertyInfo* pMaterialProperty = pMaterialData->GetPropertyInfoAt(materialManager, materialPropertyIndex);

			if (propInfo->IsResource())
			{
				auto resourceId = pMaterialData->GetResourceUUIDPointer(materialManager, pMaterialProperty->Offset());
				const std::string& sampler = propInfo->ShaderName();
				PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(ST_Asset);

				ImGui::PushID(sampler.data());
				pPropertyDrawer->Draw(pMaterialProperty->DisplayName(), resourceId, pMaterialProperty->TypeHash(), pMaterialProperty->Flags());
				ImGui::PopID();
				continue;
			}

			ImGui::PushID(propInfo->ShaderName().data());
			void* pAddress = pMaterialData->Address(materialManager, i);
			PropertyDrawer::DrawProperty(propInfo->DisplayName(), pAddress, propInfo->TypeHash(), pMaterialProperty->Flags());
			ImGui::PopID();
		}
	}
}
