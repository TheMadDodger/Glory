#include "InternalMaterial.h"

namespace Glory
{
	InternalMaterial::InternalMaterial(std::vector<FileData*>&& compiledShaders, std::vector<ShaderType>&& shaderTypes):
		m_pCompiledShaders(std::move(compiledShaders)), m_ShaderTypes(std::move(shaderTypes))
	{
	}

	size_t InternalMaterial::ShaderCount() const
	{
		return m_pCompiledShaders.size();
	}

	FileData* InternalMaterial::GetShaderAt(ShaderManager&, size_t index) const
	{
		return m_pCompiledShaders[index];
	}
}
