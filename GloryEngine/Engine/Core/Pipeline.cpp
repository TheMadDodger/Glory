#include "Pipeline.h"

namespace Glory
{
	Pipeline::Pipeline(PipelineData* pPipelineData): m_pPipelineData(pPipelineData), m_Complete(false)
	{
	}

	Pipeline::~Pipeline()
	{
		for (Shader* pShader : m_pShaders)
			delete pShader;
		m_pShaders.clear();
	}

	void Pipeline::AddShader(Shader* pShader)
	{
		m_pShaders.push_back(pShader);
	}

	void Pipeline::Clear()
	{
		for (Shader* pShader : m_pShaders)
			delete pShader;
		m_pShaders.clear();
	}
}
