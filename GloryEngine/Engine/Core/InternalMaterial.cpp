#include "InternalMaterial.h"
#include "FileData.h"

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

	ShaderType InternalMaterial::GetShaderTypeAt(ShaderManager& manager, size_t index) const
	{
		return m_ShaderTypes[index];
	}

	UUID InternalMaterial::GetShaderIDAt(size_t index) const
	{
		return m_pCompiledShaders[index]->GetUUID();
	}

	FileData* InternalMaterial::GetShaderAt(ShaderManager&, size_t index) const
	{
		return m_pCompiledShaders[index];
	}
}
