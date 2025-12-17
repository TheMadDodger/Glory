#include "InternalPipeline.h"
#include "FileData.h"

namespace Glory
{
	InternalPipeline::InternalPipeline()
	{
	}

	InternalPipeline::InternalPipeline(std::vector<FileData*>&& compiledShaders, std::vector<ShaderType>&& shaderTypes):
		m_pCompiledShaders(std::move(compiledShaders)), m_ShaderTypes(std::move(shaderTypes))
	{
	}

	InternalPipeline::~InternalPipeline()
	{
		for (size_t i = 0; i < m_pCompiledShaders.size(); ++i)
			delete m_pCompiledShaders[i];
		m_pCompiledShaders.clear();
	}

	void InternalPipeline::SetShaders(std::vector<FileData*>&& compiledShaders, std::vector<ShaderType>&& shaderTypes)
	{
		m_pCompiledShaders = std::move(compiledShaders);
		m_ShaderTypes = std::move(shaderTypes);
	}

	size_t InternalPipeline::ShaderCount() const
	{
		return m_pCompiledShaders.size();
	}

	UUID InternalPipeline::ShaderID(size_t index) const
	{
		return m_pCompiledShaders[index]->GetUUID();
	}

	ShaderType InternalPipeline::GetShaderType(const PipelineManager& manager, size_t index) const
	{
		return m_ShaderTypes[index];
	}

	FileData* InternalPipeline::Shader(const PipelineManager&, size_t index) const
	{
		return m_pCompiledShaders[index];
	}
}
