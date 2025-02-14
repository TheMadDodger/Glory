#include "EditorMaterialManager.h"
#include "EditorAssetCallbacks.h"
#include "EditorAssetDatabase.h"
#include "Importer.h"
#include "ProjectSpace.h"
#include "MaterialData.h"
#include "MaterialInstanceData.h"
#include "EditorPipelineManager.h"
#include "ResourceType.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"
#include "EditableResource.h"
#include "Dispatcher.h"
#include "EditorShaderData.h"
#include "MaterialInstanceImporter.h"
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
		m_MaterialInstances.clear();
		m_WaitingMaterialInstances.clear();
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

	void EditorMaterialManager::LoadIntoMaterial(Utils::YAMLFileRef& file, MaterialData* pMaterial) const
	{
		const UUID pipelineID = file["Pipeline"].As<uint64_t>();
		pMaterial->SetPipeline(pipelineID);
		auto properties = file["Properties"];
		ReadPropertiesInto(properties, pMaterial);
	}

	void EditorMaterialManager::LoadIntoMaterial(Utils::YAMLFileRef& file, MaterialInstanceData*& pMaterial) const
	{
		const UUID baseMaterial = file["BaseMaterial"].As<uint64_t>();
		pMaterial = new MaterialInstanceData(baseMaterial);
		ReadPropertiesInto(file["Overrides"], pMaterial);
	}

	void EditorMaterialManager::SetMaterialPipeline(UUID materialID, UUID pipelineID)
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(materialID);
		if (!pResource) return;
		MaterialData* pMaterial = static_cast<MaterialData*>(pResource);
		pMaterial->SetPipeline(pipelineID);
		YAMLResource<MaterialData>* pMaterialData = static_cast<YAMLResource<MaterialData>*>(
			EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(materialID));
		Utils::YAMLFileRef& file = **pMaterialData;
		file["Pipeline"].Set(uint64_t(pipelineID));
		UpdateMaterial(pMaterial);
	}

	void EditorMaterialManager::SetMaterialInstanceBaseMaterial(UUID materialInstanceID, UUID baseMaterialID)
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(materialInstanceID);
		if (!pResource) return;
		MaterialInstanceData* pMaterial = static_cast<MaterialInstanceData*>(pResource);

		pResource = m_pEngine->GetAssetManager().FindResource(baseMaterialID);
		if (!pResource) return;
		MaterialData* pBaseMaterial = static_cast<MaterialData*>(pResource);

		YAMLResource<MaterialInstanceData>* pMaterialInstanceData = static_cast<YAMLResource<MaterialInstanceData>*>(
			EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(materialInstanceID));
		Utils::YAMLFileRef& file = **pMaterialInstanceData;
		file["BaseMaterial"].Set(uint64_t(baseMaterialID));
		pMaterial->SetBaseMaterialID(baseMaterialID);

		ReadPropertiesInto(file["Overrides"], pBaseMaterial);
	}

	MaterialData* EditorMaterialManager::GetMaterial(UUID materialID) const
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(materialID);
		if (!pResource) return nullptr;
		return static_cast<MaterialData*>(pResource);
	}

	MaterialInstanceData* EditorMaterialManager::CreateRuntimeMaterialInstance(UUID baseMaterial)
	{
		AssetManager& asset = m_pEngine->GetAssetManager();
		Resource* pResource = asset.FindResource(baseMaterial);
		if (!pResource) return nullptr;
		MaterialData* pBaseMaterial = static_cast<MaterialData*>(pResource);
		MaterialInstanceData* pMaterialData = new MaterialInstanceData(baseMaterial);
		pMaterialData->Resize(*this, pBaseMaterial);
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
		static const size_t materialInstanceDataHash = ResourceTypes::GetHash<MaterialInstanceData>();
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

			/* Update material instances that were waiting for this material */
			auto itor = m_WaitingMaterialInstances.find(callback.m_UUID);
			if (itor == m_WaitingMaterialInstances.end()) return;
			for (size_t i = 0; i < itor->second.size(); ++i)
			{
				const UUID matInstance = itor->second[i];
				MaterialInstanceData* pMatInst = GetMaterialInstance(matInstance);
				if (!pMatInst) continue;
				MaterialInstanceImporter* pImporter = static_cast<MaterialInstanceImporter*>(Importer::GetImporter(".gminst"));
				if (!pImporter) continue;
				EditableResource* pEditResource = EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(matInstance);
				Utils::YAMLFileRef& file = **static_cast<YAMLResource<MaterialInstanceData>*>(pEditResource);
				ReadPropertiesInto(file["Overrides"], pMatInst);
			}
			itor->second.clear();
			m_WaitingMaterialInstances.erase(itor);
		}
		else if (typeHash == materialInstanceDataHash)
		{
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(callback.m_UUID);
			if (!pResource)
			{
				MaterialInstanceData* pMaterialData = nullptr;
				YAMLResource<MaterialInstanceData>* pMaterial = static_cast<YAMLResource<MaterialInstanceData>*>(resourceManager.GetEditableResource(callback.m_UUID));
				if (!pMaterial)
				{
					delete pMaterialData;
					return;
				}
				LoadIntoMaterial(**pMaterial, pMaterialData);
				pResource = pMaterialData;
				pResource->SetResourceUUID(callback.m_UUID);
				m_pEngine->GetAssetManager().AddLoadedResource(pResource);
			}

			MaterialInstanceData* pMaterial = static_cast<MaterialInstanceData*>(pResource);
			pMaterial->SetResourceUUID(callback.m_UUID);
			m_MaterialInstances.push_back(callback.m_UUID);

			/* If the base material isnt loaded we must update it when it is */
			const UUID baseMaterial = pMaterial->BaseMaterialID();
			if (!baseMaterial || !EditorAssetDatabase::AssetExists(baseMaterial)) return;

			pResource = m_pEngine->GetAssetManager().FindResource(baseMaterial);
			if (!pResource) return;
			MaterialData* pBaseMaterial = static_cast<MaterialData*>(pResource);
			if (pBaseMaterial)
			{
				pMaterial->Resize(*this, pBaseMaterial);
				return;
			}
			m_WaitingMaterialInstances[baseMaterial].push_back(callback.m_UUID);
		}
	}

	void EditorMaterialManager::AssetUpdatedCallback(const AssetCallbackData& callback)
	{
		ResourceMeta meta;
		EditorAssetDatabase::GetAssetMetadata(callback.m_UUID, meta);
		const uint32_t typeHash = meta.Hash();
		static const size_t shaderSourceDataHash = ResourceTypes::GetHash<MaterialData>();
		static const size_t materialInstanceDataHash = ResourceTypes::GetHash<MaterialInstanceData>();
		if (typeHash != shaderSourceDataHash) return;
	}

	void EditorMaterialManager::PipelineUpdateCallback(PipelineData* pPipeline)
	{
		for (const UUID materialID : m_Materials)
		{
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(materialID);
			if (!pResource) continue;
			MaterialData* pMaterial = static_cast<MaterialData*>(pResource);
			if (pMaterial->GetPipelineID(*this) != pPipeline->GetUUID()) continue;
			UpdateMaterial(pMaterial);
		}
	}

	void EditorMaterialManager::ReadPropertiesInto(Utils::NodeValueRef& properties, MaterialData* pMaterial, bool clearProperties) const
	{
		if (!properties.IsMap()) return;
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
				pMaterial->GetPropertyInfoIndex(*this, displayName, index);
				const size_t offset = pMaterial->GetPropertyInfoAt(*this, index)->Offset();
				m_pEngine->GetSerializers().DeserializeProperty(pMaterial->GetBufferReference(*this), type, offset, typeData != nullptr ? typeData->m_Size : 4, value);
			}
			else
			{
				const TextureType textureType = EditorPipelineManager::ShaderNameToTextureType(name);
				const UUID id = value.As<uint64_t>();
				pMaterial->AddResourceProperty(displayName, name, type, id, textureType);
			}
		}
	}

	void EditorMaterialManager::ReadPropertiesInto(Utils::NodeValueRef& properties, MaterialInstanceData* pMaterialData, bool clearProperties) const
	{
		MaterialManager& manager = EditorApplication::GetInstance()->GetEngine()->GetMaterialManager();

		MaterialData* baseMaterial = GetMaterial(pMaterialData->BaseMaterialID());
		if (!baseMaterial) return;
		pMaterialData->Resize(manager, baseMaterial);

		if (!properties.IsMap()) return;

		for (auto itor = properties.Begin(); itor != properties.End(); ++itor)
		{
			const std::string displayName = *itor;
			auto prop = properties[displayName];
			const bool enable = prop["Enable"].As<bool>();
			if (!prop.Exists()) continue;

			size_t propertyIndex = 0;
			if (!pMaterialData->GetPropertyInfoIndex(manager, displayName, propertyIndex)) continue;
			if (enable) pMaterialData->EnableProperty(propertyIndex);

			MaterialPropertyInfo* propertyInfo = pMaterialData->GetPropertyInfoAt(manager, propertyIndex);

			auto value = prop["Value"];

			if (!propertyInfo->IsResource())
			{
				const uint32_t typeHash = propertyInfo->TypeHash();
				const size_t offset = propertyInfo->Offset();
				const size_t size = propertyInfo->Size();
				EditorApplication::GetInstance()->GetEngine()->GetSerializers().DeserializeProperty(pMaterialData->GetBufferReference(manager), typeHash, offset, size, value);
			}
			else
			{
				if (!value.Exists() || !value.IsScalar()) continue;
				const UUID id = value.As<uint64_t>();
				size_t resourceIndex = propertyInfo->Offset();
				if (pMaterialData->ResourceCount() > resourceIndex) *pMaterialData->GetResourceUUIDPointer(manager, resourceIndex) = id;
			}
		}
 	}

	void EditorMaterialManager::UpdateMaterial(MaterialData* pMaterial)
	{
		EditorApplication* pApplication = EditorApplication::GetInstance();

		pMaterial->ClearProperties();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		PipelineData* pPipeline = pMaterial->GetPipeline(*this, pipelines);
		pPipeline->LoadIntoMaterial(pMaterial);

		EditableResource* pResource = pApplication->GetResourceManager().GetEditableResource(pMaterial->GetUUID());
		if (!pResource || !pResource->IsEditable()) return;
		YAMLResource<MaterialData>* pEditorMaterialData = static_cast<YAMLResource<MaterialData>*>(pResource);

		Utils::YAMLFileRef& file = **pEditorMaterialData;
		ReadPropertiesInto(file["Properties"], pMaterial, false);
		/* Update properties in YAML? */

		/* Find and update material instances */
		for (const UUID materialID : m_MaterialInstances)
		{
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(materialID);
			if (!pResource) continue;
			MaterialInstanceData* pMaterialInstance = static_cast<MaterialInstanceData*>(pResource);
			if (pMaterialInstance->BaseMaterialID() != pMaterial->GetUUID()) continue;

			EditableResource* pInstanceResource = pApplication->GetResourceManager().GetEditableResource(pMaterialInstance->GetUUID());
			if (!pInstanceResource || !pInstanceResource->IsEditable()) return;
			YAMLResource<MaterialInstanceData>* pEditorMaterialInstanceData = static_cast<YAMLResource<MaterialInstanceData>*>(pInstanceResource);
			Utils::YAMLFileRef& instanceFile = **pEditorMaterialInstanceData;
			ReadPropertiesInto(instanceFile["Overrides"], pMaterialInstance);
		}
	}

	MaterialInstanceData* EditorMaterialManager::GetMaterialInstance(UUID materialID) const
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(materialID);
		if (!pResource) return nullptr;
		return static_cast<MaterialInstanceData*>(pResource);
	}
}
