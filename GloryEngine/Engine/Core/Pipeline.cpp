#include "Pipeline.h"

namespace Glory
{
	Pipeline::Pipeline(PipelineData* pPipelineData): m_pPipelineData(pPipelineData), m_Complete(false)
	{
	}

	Pipeline::~Pipeline()
	{
	}

	void Pipeline::AddShader(Shader* pShader)
	{
		m_pShaders.push_back(pShader);
	}

	void Pipeline::Clear()
	{
		m_pShaders.clear();
	}
}
