#include "RuntimeMaterialManager.h"

#include <Engine.h>
#include <AssetManager.h>
#include <MaterialData.h>
#include <MaterialInstanceData.h>

namespace Glory
{
	RuntimeMaterialManager::RuntimeMaterialManager(Engine* pEngine): MaterialManager(pEngine)
	{
	}

	MaterialData* RuntimeMaterialManager::GetMaterial(UUID materialID) const
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(materialID);
		if (!pResource) return nullptr;
		MaterialData* pMaterial = static_cast<MaterialData*>(pResource);
		return pMaterial;
	}

	MaterialInstanceData* RuntimeMaterialManager::CreateRuntimeMaterialInstance(UUID baseMaterial)
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

	void RuntimeMaterialManager::DestroyRuntimeMaterials()
	{
		AssetManager& assets = m_pEngine->GetAssetManager();
		for (auto materialID : m_RuntimeMaterials)
		{
			assets.UnloadAsset(materialID);
		}
		m_RuntimeMaterials.clear();
	}
}
