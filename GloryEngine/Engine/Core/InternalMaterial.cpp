#include "InternalMaterial.h"
#include "PipelineData.h"

namespace Glory
{
	InternalMaterial::InternalMaterial(PipelineData* pPipeline): m_pPipeline(pPipeline)
	{
	}

	PipelineData* InternalMaterial::GetPipeline(const MaterialManager&, const PipelineManager& pipelineManager) const
	{
		return m_pPipeline;
	}

	UUID InternalMaterial::GetPipelineID(const MaterialManager&) const
	{
		return m_pPipeline ? m_pPipeline->GetUUID() : 0;
	}
}
