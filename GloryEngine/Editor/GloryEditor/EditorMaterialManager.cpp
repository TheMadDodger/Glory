#include "EditorMaterialManager.h"
#include "EditorAssetCallbacks.h"
#include "EditorAssetDatabase.h"
#include "Importer.h"
#include "ProjectSpace.h"
#include "MaterialData.h"
#include "MaterialInstanceData.h"
#include "EditorShaderProcessor.h"
#include "ResourceType.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"
#include "EditableResource.h"

#include <Serializers.h>
#include <NodeRef.h>

#include <Engine.h>
#include <AssetDatabase.h>
#include <AssetManager.h>

namespace Glory::Editor
{
	EditorMaterialManager::EditorMaterialManager(Engine* pEngine):
		m_pEngine(pEngine), m_AssetRegisteredCallback(0), m_AssetUpdatedCallback(0)
	{
	}

	EditorMaterialManager::~EditorMaterialManager()
	{
	}

	void EditorMaterialManager::Start()
	{
		m_AssetRegisteredCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetRegistered,
			[this](const AssetCallbackData& callback) { AssetAddedCallback(callback); });
		//m_AssetUpdatedCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetUpdated,
			//[this](const AssetCallbackData& callback) { AssetUpdatedCallback(callback); });
	}

	void EditorMaterialManager::Stop()
	{
		EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetRegisteredCallback);
		//EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetUpdatedCallback);
	}

	void EditorMaterialManager::LoadIntoMaterial(Utils::YAMLFileRef& file, MaterialData* pMaterial) const
	{
		auto shaders = file["Shaders"];
		ReadShadersInto(shaders, pMaterial);
		auto properties = file["Properties"];
		ReadPropertiesInto(shaders, pMaterial);
	}

	void EditorMaterialManager::AssetAddedCallback(const AssetCallbackData& callback)
	{
		ResourceMeta meta;
		if (!EditorAssetDatabase::GetAssetMetadata(callback.m_UUID, meta))
			return;
		AssetLocation location;
		if (!EditorAssetDatabase::GetAssetLocation(callback.m_UUID, location))
			return;

		std::filesystem::path assetPath = ProjectSpace::GetOpenProject()->RootPath();
		assetPath.append("Assets").append(location.Path);
		if (!std::filesystem::exists(assetPath))
		{
			assetPath = location.Path;
		}

		const uint32_t typeHash = meta.Hash();
		static const size_t materialDataHash = ResourceTypes::GetHash<MaterialData>();
		if (typeHash == materialDataHash)
		{
			if (m_pMaterialDatas.Contains(callback.m_UUID))
			{
				EditableResource* pEditableResource = EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(callback.m_UUID);
				if (!pEditableResource) return;
				YAMLResource<MaterialData>* pEditableMaterial = static_cast<YAMLResource<MaterialData>*>(pEditableResource);
				m_pMaterialDatas.Do(callback.m_UUID, [this, pEditableMaterial](MaterialData** pMaterial) {
					/* Overwrite the material with new data */
					LoadIntoMaterial(**pEditableMaterial, *pMaterial);
				});
			}

			/* Immediately import the material, which is fast */
			Resource* pResource = Importer::Import(assetPath, nullptr);
			if (!pResource) return;
			MaterialData* pMaterial = static_cast<MaterialData*>(pResource);
			pMaterial->SetResourceUUID(callback.m_UUID);
			m_pEngine->GetAssetManager().AddLoadedResource(pMaterial);
			m_pMaterialDatas.Set(callback.m_UUID, pMaterial);
		}
	}

	void EditorMaterialManager::AssetUpdatedCallback(const AssetCallbackData& callback)
	{
		ResourceMeta meta;
		EditorAssetDatabase::GetAssetMetadata(callback.m_UUID, meta);
		const uint32_t typeHash = meta.Hash();
		static const size_t shaderSourceDataHash = ResourceTypes::GetHash<MaterialData>();
		if (typeHash != shaderSourceDataHash) return;
	}

	void EditorMaterialManager::ReadShadersInto(Utils::NodeValueRef& shaders, MaterialData* pMaterial) const
	{
		pMaterial->RemoveAllShaders();
		if (!shaders.IsSequence()) return;
		for (size_t i = 0; i < shaders.Size(); ++i)
		{
			auto shader = shaders[i];
			const UUID shaderID = shader["UUID"].As<uint64_t>();

			AssetLocation location;
			if (!EditorAssetDatabase::GetAssetLocation(shaderID, location)) continue;

			const std::string_view assetPath = m_pEngine->GetAssetDatabase().GetAssetPath();
			const std::string path = std::string{ assetPath } + '\\' + location.Path;
			ShaderSourceData* pShaderSourceData = EditorShaderProcessor::GetShaderSource(shaderID);
			if (!pShaderSourceData) continue;
			pMaterial->AddShader(pShaderSourceData);
		}
	}

	void EditorMaterialManager::ReadPropertiesInto(Utils::NodeValueRef& properties, MaterialData* pMaterial) const
	{
		if (!properties.IsMap()) return;

		pMaterial->ClearProperties();
		for (auto itor = properties.Begin(); itor != properties.End(); ++itor)
		{
			const std::string name = *itor;
			auto prop = properties[name];
			const std::string displayName = prop["DisplayName"].As<std::string>();
			const uint32_t type = prop["TypeHash"].As<uint32_t>();
			auto value = prop["Value"];

			const BasicTypeData* typeData = m_pEngine->GetResourceTypes().GetBasicTypeData(type);

			const size_t offset = pMaterial->GetCurrentBufferOffset();

			bool isResource = m_pEngine->GetResourceTypes().IsResource(type);
			if (!isResource)
			{
				pMaterial->AddProperty(displayName, name, type, typeData != nullptr ? typeData->m_Size : 4, 0);
				m_pEngine->GetSerializers().DeserializeProperty(pMaterial->GetBufferReference(), type, offset, typeData != nullptr ? typeData->m_Size : 4, value.Node());
			}
			else
			{
				const UUID id = value.As<uint64_t>();
				pMaterial->AddProperty(displayName, name, type, id);
			}
		}
	}
}
