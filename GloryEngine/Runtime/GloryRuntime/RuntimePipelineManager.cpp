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
		PipelineData* pPipeline = static_cast<PipelineData*>(pResource);
		return pPipeline;
	}

	const std::vector<FileData>& RuntimePipelineManager::GetPipelineCompiledShaders(UUID pipelineID) const
	{
		return m_Shaders.at(pipelineID);
	}

	const std::vector<ShaderType>& RuntimePipelineManager::GetPipelineShaderTypes(UUID pipelineID) const
	{
		return m_ShaderTypes.at(pipelineID);
	}

	void RuntimePipelineManager::AddShader(FileData* pShader)
	{
		const PipelineShaderMetaData& metaData = pShader->GetMetaData<PipelineShaderMetaData>();
		m_Shaders[metaData.PipelineID].push_back(std::move(*pShader));
		m_ShaderTypes[metaData.PipelineID].push_back(metaData.Type);
	}
}
