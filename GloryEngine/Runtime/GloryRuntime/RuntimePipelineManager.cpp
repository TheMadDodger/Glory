#include "RuntimePipelineManager.h"

#include <Engine.h>
#include <AssetManager.h>
#include <PipelineData.h>

namespace Glory
{
	RuntimePipelineManager::RuntimePipelineManager(Engine* pEngine): PipelineManager(pEngine)
	{
	}

	PipelineData* RuntimePipelineManager::GetPipelineData(UUID pipelineID) const
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(pipelineID);
		if (!pResource) return nullptr;
		PipelineData* pMaterial = static_cast<PipelineData*>(pResource);
		return pMaterial;
	}
}
