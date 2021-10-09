#include "MaterialData.h"

namespace Glory
{
	MaterialData::MaterialData(const std::vector<FileData*>& shaderFiles, const std::vector<ShaderType>& shaderTypes)
		: m_pShaderFiles(shaderFiles), m_ShaderTypes(shaderTypes)
	{
		
	}

	MaterialData::~MaterialData()
	{
		m_pShaderFiles.clear();
		m_ShaderTypes.clear();
	}

	size_t MaterialData::ShaderCount() const
	{
		return m_pShaderFiles.size();
	}

	FileData* MaterialData::GetShaderAt(size_t index) const
	{
		return m_pShaderFiles[index];
	}

	const ShaderType& MaterialData::GetShaderTypeAt(size_t index) const
	{
		return m_ShaderTypes[index];
	}

	void MaterialData::SetTexture(ImageData* pTexture)
	{
		m_pTexture = pTexture;
	}

	ImageData* MaterialData::GetTexture()
	{
		return m_pTexture;
	}
}
