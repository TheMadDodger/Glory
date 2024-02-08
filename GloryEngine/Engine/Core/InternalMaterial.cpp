#include "InternalMaterial.h"
#include "FileData.h"

namespace Glory
{
	InternalMaterial::InternalMaterial(std::vector<FileData*>&& compiledShaders, std::vector<ShaderType>&& shaderTypes):
		m_pCompiledShaders(std::move(compiledShaders)), m_ShaderTypes(std::move(shaderTypes))
	{
	}

	size_t InternalMaterial::ShaderCount(const MaterialManager&) const
	{
		return m_pCompiledShaders.size();
	}

	ShaderType InternalMaterial::GetShaderTypeAt(const MaterialManager&, ShaderManager& manager, size_t index) const
	{
		return m_ShaderTypes[index];
	}

	UUID InternalMaterial::GetShaderIDAt(const MaterialManager&, size_t index) const
	{
		return m_pCompiledShaders[index]->GetUUID();
	}

	FileData* InternalMaterial::GetShaderAt(const MaterialManager&, ShaderManager&, size_t index) const
	{
		return m_pCompiledShaders[index];
	}
}
