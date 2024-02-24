#include "RuntimeMaterialManager.h"

#include <Engine.h>
#include <AssetManager.h>
#include <MaterialData.h>

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
}
