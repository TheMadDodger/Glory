#include "InternalPipeline.h"
#include "FileData.h"

namespace Glory
{
	InternalPipeline::InternalPipeline(std::vector<FileData*>&& compiledShaders, std::vector<ShaderType>&& shaderTypes):
		m_pCompiledShaders(std::move(compiledShaders)), m_ShaderTypes(std::move(shaderTypes))
	{
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
