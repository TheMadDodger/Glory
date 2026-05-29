#include "EditorMaterialManager.h"
#include "EditorAssetCallbacks.h"
#include "EditorAssetDatabase.h"
#include "Importer.h"
#include "ProjectSpace.h"
#include "EditorPipelineManager.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"
#include "EditableResource.h"
#include "Dispatcher.h"
#include "EditorShaderData.h"

#include <IEngine.h>
#include <PipelineData.h>
#include <Serializers.h>
#include <AssetDatabase.h>
#include <Resources.h>
#include <MaterialData.h>
#include <ResourceType.h>

namespace Glory::Editor
{
	EditorMaterialManager::EditorMaterialManager(EditorApplication* pApp):
		m_pApplication(pApp), m_AssetRegisteredCallback(0),
		m_PipelineUpdatedCallback(0), MaterialManager(pApp->GetEngine())
	{
	}

	EditorMaterialManager::~EditorMaterialManager()
	{
		m_MaterialsPerPipeline.clear();
	}

	void EditorMaterialManager::Initialize()
	{
		m_AssetRegisteredCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetRegistered,
			[this](const AssetCallbackData& callback) { AssetAddedCallback(callback); });
		m_PipelineUpdatedCallback = EditorApplication::GetInstance()->GetPipelineManager().PipelineUpdateEvents().AddListener([this](const PipelineUpdateEvent& e) {
			PipelineUpdateCallback(e.pPipeline);
		});
	}

	void EditorMaterialManager::Cleanup()
	{
		EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetRegisteredCallback);
		EditorApplication::GetInstance()->GetPipelineManager().PipelineUpdateEvents().RemoveListener(m_PipelineUpdatedCallback);
	}

	void EditorMaterialManager::SetMaterialPipeline(MaterialData* pMaterial, UUID materialID, UUID pipelineID)
	{
		YAMLResourceBase* pMaterialData = static_cast<YAMLResourceBase*>(
			EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(materialID));
		Utils::NodeValueRef node = **pMaterialData;

		const UUID originalPipelineID = pMaterial ? pMaterial->GetPipelineID() : UUID(node["Pipeline"].As<uint64_t>());
		if (originalPipelineID == pipelineID) return;

		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		PipelineData* pPipeline = pipelines.GetPipelineData(pipelineID);

		auto iter = m_MaterialsPerPipeline.find(originalPipelineID);
		if (iter != m_MaterialsPerPipeline.end())
		{
			auto removeIter = std::remove(iter->second.begin(), iter->second.end(), materialID);
			iter->second.erase(removeIter, iter->second.end());
		}

		node["Pipeline"].Set(uint64_t(pipelineID));
		if (!pipelineID || !pPipeline)
		{
			if (pMaterial)
			{
				pMaterial->SetPipeline(pipelineID);
				pMaterial->ClearProperties();
				pMaterial->IncrementDirtyVersion();
			}
			return;
		}

		m_MaterialsPerPipeline[pipelineID].emplace_back(materialID);

		if (!pMaterial)
		{
			MaterialData tempNewMaterial;
			pPipeline->LoadIntoMaterial(&tempNewMaterial);

			/* Load into a temporary material */
			MaterialData tempMaterial;
			std::vector<UUID> tempResources;
			LoadTemporary(node["Properties"], &tempMaterial, tempResources);

			auto& sourceBuffer = tempMaterial.GetBufferReference();
			auto& destinationBuffer = tempNewMaterial.GetBufferReference();

			/* Overwrite the properties with the ones from the original material,
			 * except resources because again, we don't want to invoke references. */
			for (size_t i = 0; i < tempNewMaterial.PropertyInfoCount(); ++i)
			{
				const MaterialPropertyInfo* pPropInfo = tempNewMaterial.GetPropertyInfoAt(i);
				if (pPropInfo->IsResource()) continue;

				/* Find same property in original material */
				size_t index = 0;
				if (!tempMaterial.GetPropertyInfoIndex(pPropInfo->DisplayName(), index)) continue;
				const MaterialPropertyInfo* pOtherPropInfo = tempMaterial.GetPropertyInfoAt(index);
				if (pPropInfo->TypeHash() != pOtherPropInfo->TypeHash()) continue;
				std::memcpy(&destinationBuffer[pPropInfo->Offset()], &sourceBuffer[pOtherPropInfo->Offset()], pPropInfo->Size());
			}

			/* Overwrite YAML data */
			WritePropertiesTo(node["Properties"], &tempNewMaterial);

			/* Overwrite resources separately */
			for (size_t i = 0; i < tempNewMaterial.GetResourcePropertyCount(); ++i)
			{
				const MaterialPropertyInfo* pPropInfo = tempNewMaterial.GetResourcePropertyInfo(i);

				/* Find same property in original material */
				size_t index = 0;
				if (!tempMaterial.GetPropertyInfoIndex(pPropInfo->DisplayName(), index)) continue;
				const MaterialPropertyInfo* pOtherPropInfo = tempMaterial.GetPropertyInfoAt(index);

				const uint64_t resourceID = tempResources[pOtherPropInfo->Offset()];
				if (!resourceID) continue;

				node["Properties"][pPropInfo->DisplayName()]["Value"].Set(resourceID);
			}
			return;
		}

		pMaterial->SetPipeline(pipelineID);
		pMaterial->IncrementDirtyVersion();
		pPipeline->LoadIntoMaterial(pMaterial);

		ReadPropertiesInto(node["Properties"], pMaterial);
		/* Update properties in YAML */
		WritePropertiesTo(node["Properties"], pMaterial);
	}

	MaterialData* EditorMaterialManager::GetMaterial(UUID materialID) const
	{
		Resource* pResource = m_pEngine->GetResources().GetResource(materialID);
		if (!pResource) return nullptr;
		return static_cast<MaterialData*>(pResource);
	}

	MaterialData* EditorMaterialManager::CreateRuntimeMaterial(UUID baseMaterial)
	{
		Resources& resources = m_pEngine->GetResources();
		Resource* pResource = resources.GetResource(baseMaterial);
		if (!pResource) return nullptr;
		MaterialData* pBaseMaterial = static_cast<MaterialData*>(pResource);
		MaterialData* pMaterialData = pBaseMaterial->CreateCopy();
		resources.AddResource(&pMaterialData);
		m_RuntimeMaterials.push_back(pMaterialData->GetUUID());
		return pMaterialData;
	}

	void EditorMaterialManager::DestroyRuntimeMaterials()
	{
		Resources& resources = m_pEngine->GetResources();
		for (auto materialID : m_RuntimeMaterials)
		{
			resources.UnloadResource(materialID);
		}
		m_RuntimeMaterials.clear();
	}

	void EditorMaterialManager::ReadPropertiesInto(Utils::NodeValueRef properties, MaterialData* pMaterial) const
	{
		if (!properties.Exists() || !properties.IsMap()) return;

		for (auto itor = properties.Begin(); itor != properties.End(); ++itor)
		{
			const std::string name = *itor;
			auto prop = properties[name];
			const std::string displayName = prop["DisplayName"].Exists() ? prop["DisplayName"].As<std::string>() : name;
			const uint32_t type = prop["TypeHash"].As<uint32_t>();
			auto value = prop["Value"];

			size_t index = 0;
			if (!pMaterial->GetPropertyInfoIndex(displayName, index)) continue;

			const BasicTypeData* typeData = m_pEngine->GetResourceTypes().GetBasicTypeData(type);

			bool isResource = m_pEngine->GetResourceTypes().IsResource(type);
			if (!isResource)
			{
				const size_t offset = pMaterial->GetPropertyInfoAt(index)->Offset();
				m_pApplication->GetSerializers().DeserializeProperty(pMaterial->GetBufferReference(), type, offset, typeData != nullptr ? typeData->m_Size : 4, value);
			}
			else
			{
				const TextureType textureType = EditorPipelineManager::ShaderNameToTextureType(name);
				const UUID id = value.As<uint64_t>();
				pMaterial->AddResourceProperty(displayName, name, type, id, textureType);
			}
		}
	}

	void EditorMaterialManager::WritePropertiesTo(Utils::NodeValueRef properties, MaterialData* pMaterial) const
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
				m_pApplication->GetSerializers().SerializeProperty(pMaterial->GetBufferReference(), type, offset, typeData != nullptr ? typeData->m_Size : 4, value);
			}
			else
			{
				const UUID id = pMaterial->GetResourceUUIDPointer(propInfo->Offset())->GetUUID();
				value.Set(uint64_t(id));
			}
		}
	}

	void EditorMaterialManager::AssetAddedCallback(const AssetCallbackData& callback)
	{
		ResourceMeta meta;
		if (!EditorAssetDatabase::GetAssetMetadata(callback.m_UUID, meta))
			return;
		AssetLocation location;
		if (!EditorAssetDatabase::GetAssetLocation(callback.m_UUID, location))
			return;

		if (!location.SubresourcePath.empty())
		{
			m_SubResourceMaterials.emplace_back(callback.m_UUID);
			return;
		}

		std::filesystem::path assetPath = ProjectSpace::GetOpenProject()->RootPath();
		assetPath.append("Assets").append(location.Path);
		if (!std::filesystem::exists(assetPath))
			assetPath = location.Path;

		EditorResourceManager& resourceManager = EditorApplication::GetInstance()->GetResourceManager();

		const uint32_t typeHash = meta.Hash();
		static const size_t materialDataHash = ResourceTypes::GetHash<MaterialData>();
		if (typeHash != materialDataHash) return;

		EditableResource* pMaterialResource = resourceManager.GetEditableResource(callback.m_UUID);
		YAMLResourceBase* pMaterial = static_cast<YAMLResourceBase*>(pMaterialResource);
		auto file = **pMaterial;
		const UUID pipelineID = file["Pipeline"].As<uint64_t>(0ull);
		if (!pipelineID) return;
		m_MaterialsPerPipeline[pipelineID].emplace_back(callback.m_UUID);
	}

	void EditorMaterialManager::PipelineUpdateCallback(PipelineData* pPipeline)
	{
		auto iter = m_MaterialsPerPipeline.find(pPipeline->GetUUID());
		if (iter == m_MaterialsPerPipeline.end()) return;

		EditorResourceManager& resourceManager = EditorApplication::GetInstance()->GetResourceManager();
		Resources& resources = EditorApplication::GetInstance()->GetEngine()->GetResources();
		for (auto materialID : iter->second)
		{
			EditableResource* pMaterialResource = resourceManager.GetEditableResource(materialID);
			YAMLResourceBase* pMaterial = static_cast<YAMLResourceBase*>(pMaterialResource);
			auto file = **pMaterial;

			/* Store the original data in a temporary material,
			 * store resources seperately because we dont want to invoke references. */
			MaterialData tempMaterial;
			std::vector<UUID> tempResources;
			LoadTemporary(file["Properties"], &tempMaterial, tempResources);

			/* Now we create a brand new material using the pipeline. */
			MaterialData tempNewMaterial;
			pPipeline->LoadIntoMaterial(&tempNewMaterial);

			auto& sourceBuffer = tempMaterial.GetBufferReference();
			auto& destinationBuffer = tempNewMaterial.GetBufferReference();

			/* Overwrite the properties with the ones from the original material,
			 * except resources because again, we don't want to invoke references. */
			for (size_t i = 0; i < tempNewMaterial.PropertyInfoCount(); ++i)
			{
				const MaterialPropertyInfo* pPropInfo = tempNewMaterial.GetPropertyInfoAt(i);
				if (pPropInfo->IsResource()) continue;
				
				/* Find same property in original material */
				size_t index = 0;
				if (!tempMaterial.GetPropertyInfoIndex(pPropInfo->DisplayName(), index)) continue;
				const MaterialPropertyInfo* pOtherPropInfo = tempMaterial.GetPropertyInfoAt(index);
				if (pPropInfo->TypeHash() != pOtherPropInfo->TypeHash()) continue;
				std::memcpy(&destinationBuffer[pPropInfo->Offset()], &sourceBuffer[pOtherPropInfo->Offset()], pPropInfo->Size());
			}

			/* Overwrite YAML data */
			WritePropertiesTo(file["Properties"], &tempNewMaterial);

			/* Overwrite resources separately */
			for (size_t i = 0; i < tempNewMaterial.GetResourcePropertyCount(); ++i)
			{
				const MaterialPropertyInfo* pPropInfo = tempNewMaterial.GetResourcePropertyInfo(i);

				/* Find same property in original material */
				size_t index = 0;
				if (!tempMaterial.GetPropertyInfoIndex(pPropInfo->DisplayName(), index)) continue;
				const MaterialPropertyInfo* pOtherPropInfo = tempMaterial.GetPropertyInfoAt(index);

				const uint64_t resourceID = tempResources[pOtherPropInfo->Offset()];
				if (!resourceID) continue;

				file["Properties"][pPropInfo->DisplayName()]["Value"].Set(resourceID);
			}

			/* Update the material if it is in memory */
			MaterialData* pMaterialData = resources.GetResource<MaterialData>(materialID);
			if (!pMaterialData) continue;
			ReadPropertiesInto(file["Properties"], pMaterialData);
		}

		/* Sub resources should be updated directly if they are in memory */
		for (auto materialID : m_SubResourceMaterials)
		{
			MaterialData* pMaterialData = resources.GetResource<MaterialData>(materialID);
			if (!pMaterialData || pMaterialData->GetPipelineID() != pPipeline->GetUUID()) continue;

			/* Serialize the original materials properties */
			Utils::InMemoryYAML data;
			WritePropertiesTo(data, pMaterialData);

			/* Reset material */
			pPipeline->LoadIntoMaterial(pMaterialData);
			ReadPropertiesInto(data, pMaterialData);
		}
	}

	void EditorMaterialManager::LoadTemporary(Utils::NodeValueRef properties, MaterialData* pMaterial, std::vector<UUID>& resources) const
	{
		if (!properties.Exists() || !properties.IsMap()) return;

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
				m_pApplication->GetSerializers().DeserializeProperty(pMaterial->GetBufferReference(),
					type, offset, typeData != nullptr ? typeData->m_Size : 4, value);
			}
			else
			{
				const TextureType textureType = EditorPipelineManager::ShaderNameToTextureType(name);
				const UUID id = value.As<uint64_t>();
				resources.emplace_back(id);
				pMaterial->AddResourceProperty(displayName, name, type, 0ull, textureType);
			}
		}
	}
}
