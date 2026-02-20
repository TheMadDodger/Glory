#include "EditorMaterialManager.h"
#include "EditorAssetCallbacks.h"
#include "EditorAssetDatabase.h"
#include "Importer.h"
#include "ProjectSpace.h"
#include "MaterialData.h"
#include "EditorPipelineManager.h"
#include "ResourceType.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"
#include "EditableResource.h"
#include "Dispatcher.h"
#include "EditorShaderData.h"
#include "AssetManager.h"

#include <PipelineData.h>
#include <Serializers.h>
#include <NodeRef.h>

#include <Engine.h>
#include <AssetDatabase.h>
#include <AssetManager.h>

namespace Glory::Editor
{
	EditorMaterialManager::EditorMaterialManager(Engine* pEngine):
		m_pEngine(pEngine), m_AssetRegisteredCallback(0), m_AssetUpdatedCallback(0),
		m_PipelineUpdatedCallback(0), MaterialManager(pEngine)
	{
	}

	EditorMaterialManager::~EditorMaterialManager()
	{
		m_Materials.clear();
	}

	void EditorMaterialManager::Initialize()
	{
		m_AssetRegisteredCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetRegistered,
			[this](const AssetCallbackData& callback) { AssetAddedCallback(callback); });
		//m_AssetUpdatedCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetUpdated,
			//[this](const AssetCallbackData& callback) { AssetUpdatedCallback(callback); });

		m_PipelineUpdatedCallback = EditorApplication::GetInstance()->GetPipelineManager().PipelineUpdateEvents().AddListener([this](const PipelineUpdateEvent& e) {
			PipelineUpdateCallback(e.pPipeline);
		});
	}

	void EditorMaterialManager::Cleanup()
	{
		EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetRegisteredCallback);
		//EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetUpdatedCallback);

		EditorApplication::GetInstance()->GetPipelineManager().PipelineUpdateEvents().RemoveListener(m_PipelineUpdatedCallback);
	}

	void EditorMaterialManager::LoadIntoMaterial(Utils::NodeValueRef node, MaterialData* pMaterial, bool clearProperties) const
	{
		const UUID pipelineID = node["Pipeline"].As<uint64_t>(pMaterial->GetPipelineID());
		pMaterial->SetPipeline(pipelineID);
		auto properties = node["Properties"];
		ReadPropertiesInto(properties, pMaterial, clearProperties);
	}

	void EditorMaterialManager::SetMaterialPipeline(UUID materialID, UUID pipelineID)
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(materialID);
		if (!pResource) return;
		MaterialData* pMaterial = static_cast<MaterialData*>(pResource);
		pMaterial->SetPipeline(pipelineID);
		pMaterial->SetDirty(true);
		YAMLResource<MaterialData>* pMaterialData = static_cast<YAMLResource<MaterialData>*>(
			EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(materialID));
		Utils::NodeValueRef node = **pMaterialData;
		node["Pipeline"].Set(uint64_t(pipelineID));
		UpdateMaterial(pMaterial);
	}

	MaterialData* EditorMaterialManager::GetMaterial(UUID materialID) const
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(materialID);
		if (!pResource) return nullptr;
		return static_cast<MaterialData*>(pResource);
	}

	MaterialData* EditorMaterialManager::CreateRuntimeMaterial(UUID baseMaterial)
	{
		AssetManager& asset = m_pEngine->GetAssetManager();
		Resource* pResource = asset.FindResource(baseMaterial);
		if (!pResource) return nullptr;
		MaterialData* pBaseMaterial = static_cast<MaterialData*>(pResource);
		MaterialData* pMaterialData = pBaseMaterial->CreateCopy();
		m_RuntimeMaterials.push_back(pMaterialData->GetUUID());
		asset.AddLoadedResource(pMaterialData);
		return pMaterialData;
	}

	void EditorMaterialManager::DestroyRuntimeMaterials()
	{
		AssetManager& assets = m_pEngine->GetAssetManager();
		for (auto materialID : m_RuntimeMaterials)
		{
			assets.UnloadAsset(materialID);
		}
		m_RuntimeMaterials.clear();
	}

	void EditorMaterialManager::LoadMaterial(UUID materialID)
	{
		AssetCallbackData data;
		data.m_UUID = materialID;
		AssetAddedCallback(data);
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

		EditorResourceManager& resourceManager = EditorApplication::GetInstance()->GetResourceManager();

		const uint32_t typeHash = meta.Hash();
		static const size_t materialDataHash = ResourceTypes::GetHash<MaterialData>();
		if (typeHash == materialDataHash)
		{
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(callback.m_UUID);
			if (!pResource)
			{
				MaterialData* pMaterialData = new MaterialData();
				pResource = pMaterialData;
				pResource->SetResourceUUID(callback.m_UUID);
				if (!location.SubresourcePath.empty())
				{
					delete pMaterialData;
					return;
				}
				EditableResource* pMaterialResource = resourceManager.GetEditableResource(callback.m_UUID);
				YAMLResource<MaterialData>* pMaterial = static_cast<YAMLResource<MaterialData>*>(pMaterialResource);
				LoadIntoMaterial(**pMaterial, pMaterialData);
				m_pEngine->GetAssetManager().AddLoadedResource(pResource);
			}

			MaterialData* pMaterial = static_cast<MaterialData*>(pResource);
			pMaterial->SetResourceUUID(callback.m_UUID);
			m_Materials.push_back(callback.m_UUID);
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

	void EditorMaterialManager::PipelineUpdateCallback(PipelineData* pPipeline)
	{
		for (const UUID materialID : m_Materials)
		{
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(materialID);
			if (!pResource) continue;
			MaterialData* pMaterial = static_cast<MaterialData*>(pResource);
			if (pMaterial->GetPipelineID() != pPipeline->GetUUID()) continue;
			UpdateMaterial(pMaterial);
		}
	}

	void EditorMaterialManager::ReadPropertiesInto(Utils::NodeValueRef& properties, MaterialData* pMaterial, bool clearProperties) const
	{
		if (!properties.Exists() || !properties.IsMap()) return;
		if (clearProperties) pMaterial->ClearProperties();

		for (auto itor = properties.Begin(); itor != properties.End(); ++itor)
		{
			const std::string name = *itor;
			auto prop = properties[name];
			const std::string displayName = prop["DisplayName"].Exists() ? prop["DisplayName"].As<std::string>() : name;
			const uint32_t type = prop["TypeHash"].As<uint32_t>();
			auto value = prop["Value"];

			const BasicTypeData* typeData = m_pEngine->GetResourceTypes().GetBasicTypeData(type);

			bool isResource = m_pEngine->GetResourceTypes().IsResource(type);
			if (!isResource)
			{
				pMaterial->AddProperty(displayName, name, type, typeData != nullptr ? typeData->m_Size : 4, 0);
				size_t index = 0;
				pMaterial->GetPropertyInfoIndex(displayName, index);
				const size_t offset = pMaterial->GetPropertyInfoAt(index)->Offset();
				m_pEngine->GetSerializers().DeserializeProperty(pMaterial->GetBufferReference(), type, offset, typeData != nullptr ? typeData->m_Size : 4, value);
			}
			else
			{
				const TextureType textureType = EditorPipelineManager::ShaderNameToTextureType(name);
				const UUID id = value.As<uint64_t>();
				pMaterial->AddResourceProperty(displayName, name, type, id, textureType);
			}
		}
	}

	void EditorMaterialManager::WritePropertiesTo(Utils::NodeValueRef& properties, MaterialData* pMaterial)
	{
		if (!properties.IsMap()) properties.SetMap();

		for (size_t i = 0; i < pMaterial->PropertyInfoCount(); ++i)
		{
			const MaterialPropertyInfo* propInfo = pMaterial->GetPropertyInfoAt(i);
			const uint32_t type = propInfo->TypeHash();
			const BasicTypeData* typeData = m_pEngine->GetResourceTypes().GetBasicTypeData(type);

			const std::string_view name = propInfo->ShaderName();
			auto prop = properties[name];
			if (!prop.IsMap()) prop.SetMap();
			prop["DisplayName"].Set(propInfo->ShaderName());
			prop["TypeHash"].Set(propInfo->TypeHash());
			auto value = prop["Value"];

			const bool isResource = propInfo->IsResource();
			if (!isResource)
			{
				const size_t offset = propInfo->Offset();
				m_pEngine->GetSerializers().SerializeProperty(pMaterial->GetBufferReference(), type, offset, typeData != nullptr ? typeData->m_Size : 4, value);
			}
			else
			{
				const UUID id = pMaterial->GetResourceUUIDPointer(propInfo->Offset())->AssetUUID();
				value.Set(uint64_t(id));
			}
		}
	}

	void EditorMaterialManager::UpdateMaterial(MaterialData* pMaterial)
	{
		EditorApplication* pApplication = EditorApplication::GetInstance();

		pMaterial->ClearProperties();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		PipelineData* pPipeline = pMaterial->GetPipeline(pipelines);
		pPipeline->LoadIntoMaterial(pMaterial);

		EditableResource* pResource = pApplication->GetResourceManager().GetEditableResource(pMaterial->GetUUID());
		if (!pResource || !pResource->IsEditable()) return;
		YAMLResource<MaterialData>* pEditorMaterialData = static_cast<YAMLResource<MaterialData>*>(pResource);
		Utils::NodeValueRef node = **pEditorMaterialData;

		ReadPropertiesInto(node["Properties"], pMaterial, false);
		/* Update properties in YAML */
		WritePropertiesTo(node["Properties"], pMaterial);
	}
}
