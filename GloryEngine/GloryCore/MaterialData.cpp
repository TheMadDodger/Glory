#include "MaterialData.h"
#include <algorithm>

namespace Glory
{
	std::hash<std::string> MaterialData::m_Hasher = std::hash<std::string>();

	MaterialData::MaterialData()
	{
		APPEND_TYPE(MaterialData);
	}

	MaterialData::MaterialData(const std::vector<ShaderSourceData*>& shaderFiles)
		: m_pShaderFiles(shaderFiles)
	{
		APPEND_TYPE(MaterialData);
	}

	MaterialData::~MaterialData()
	{
		m_pShaderFiles.clear();
	}

	size_t MaterialData::ShaderCount() const
	{
		return m_pShaderFiles.size();
	}

	ShaderSourceData* MaterialData::GetShaderAt(size_t index) const
	{
		return m_pShaderFiles[index];
	}

	const ShaderType& MaterialData::GetShaderTypeAt(size_t index) const
	{
		return m_pShaderFiles[index]->GetShaderType();
	}

	void MaterialData::RemoveShaderAt(size_t index)
	{
		m_pShaderFiles.erase(m_pShaderFiles.begin() + index);
	}

	void MaterialData::AddShader(ShaderSourceData* pShaderSourceData)
	{
		auto it = std::find(m_pShaderFiles.begin(), m_pShaderFiles.end(), pShaderSourceData);
		if (it != m_pShaderFiles.end()) return;
		m_pShaderFiles.push_back(pShaderSourceData);
	}

	void MaterialData::AddProperty(const std::string& displayName, const std::string& shaderName, size_t typeHash, size_t size, uint32_t flags)
	{
		size_t hash = m_Hasher(displayName);
		size_t index = m_PropertyInfos.size();
		size_t lastIndex = index - 1;
		size_t offset = index > 0 ? m_PropertyInfos[lastIndex].EndOffset() : 0;
		m_PropertyInfos.push_back(MaterialPropertyInfo(displayName, shaderName, typeHash, size, offset, flags));
		m_PropertyInfos[index].Reserve(m_PropertyBuffer);
		m_HashToPropertyInfoIndex[hash] = index;
	}

	size_t MaterialData::PropertyInfoCount() const
	{
		return m_PropertyInfos.size();
	}

	const MaterialPropertyInfo& MaterialData::GetPropertyInfoAt(size_t index) const
	{
		return m_PropertyInfos[index];
	}

	size_t MaterialData::GetCurrentBufferOffset() const
	{
		size_t size = m_PropertyInfos.size();
		return size > 0 ? m_PropertyInfos[size - 1].EndOffset() : 0;
	}

	std::vector<char>& MaterialData::GetBufferReference()
	{
		return m_PropertyBuffer;
	}

	std::vector<char>& MaterialData::GetFinalBufferReference()
	{
		return m_PropertyBuffer;
	}

	bool MaterialData::GetPropertyInfoIndex(const std::string& name, size_t& index) const
	{
		size_t hash = m_Hasher(name);
		if (m_HashToPropertyInfoIndex.find(hash) == m_HashToPropertyInfoIndex.end()) return false;
		index = m_HashToPropertyInfoIndex.at(hash);
		return true;
	}

	void MaterialData::ReloadResourcesFromShader()
	{
		for (size_t i = 0; i < m_pShaderFiles.size(); i++)
		{
			ShaderSourceData* pShaderSource = m_pShaderFiles[i];
			const spirv_cross::ShaderResources resources = pShaderSource->GetResources();
			for (size_t i = 0; i < resources.sampled_images.size(); i++)
			{
				spirv_cross::Resource sampler = resources.sampled_images[i];
				 sampler.name;
			}
		}
	}
}
