#include "RuntimeMaterialManager.h"

namespace Glory
{
	RuntimeMaterialManager::RuntimeMaterialManager(Engine* pEngine): MaterialManager(pEngine)
	{
	}

	MaterialData* RuntimeMaterialManager::GetMaterial(UUID materialID) const
	{
		return nullptr;
	}
}
