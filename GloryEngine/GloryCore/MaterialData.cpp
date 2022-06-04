#include "MaterialData.h"
#include <algorithm>
#include "ResourceType.h"

namespace Glory
{
	std::hash<std::string> MaterialData::m_Hasher = std::hash<std::string>();

	MaterialData::MaterialData()
		: m_CurrentOffset(0)
	{
		APPEND_TYPE(MaterialData);
	}

	MaterialData::MaterialData(const std::vector<ShaderSourceData*>& shaderFiles)
		: m_pShaderFiles(shaderFiles), m_CurrentOffset(0)
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

	bool MaterialData::AddShader(ShaderSourceData* pShaderSourceData)
	{
		auto it = std::find(m_pShaderFiles.begin(), m_pShaderFiles.end(), pShaderSourceData);
		if (it != m_pShaderFiles.end()) return false;
		m_pShaderFiles.push_back(pShaderSourceData);
		return true;
	}

	void MaterialData::AddProperty(const std::string& displayName, const std::string& shaderName, size_t typeHash, size_t size, bool isResource, uint32_t flags)
	{
		size_t hash = m_Hasher(displayName);
		size_t index = m_PropertyInfos.size();
		size_t lastIndex = index - 1;
		m_PropertyInfos.push_back(MaterialPropertyInfo(displayName, shaderName, typeHash, size, m_CurrentOffset, flags));
		m_CurrentOffset = m_PropertyInfos[index].EndOffset();
		m_PropertyInfos[index].Reserve(m_PropertyBuffer);
		m_HashToPropertyInfoIndex[hash] = index;
	}

	void MaterialData::AddProperty(const std::string& displayName, const std::string& shaderName, size_t typeHash, UUID resourceUUID, uint32_t flags)
	{
		size_t hash = m_Hasher(displayName);
		size_t index = m_PropertyInfos.size();
		size_t lastIndex = index - 1;
		m_PropertyInfos.push_back(MaterialPropertyInfo(displayName, shaderName, typeHash, m_Resources.size(), flags));
		m_ResourcePropertyInfoIndices.push_back(index);
		m_HashToPropertyInfoIndex[hash] = index;
		m_Resources.push_back(resourceUUID);
	}

	size_t MaterialData::PropertyInfoCount() const
	{
		return m_PropertyInfos.size();
	}

	MaterialPropertyInfo* MaterialData::GetPropertyInfoAt(size_t index)
	{
		return &m_PropertyInfos[index];
	}

	size_t MaterialData::GetCurrentBufferOffset() const
	{
		return m_CurrentOffset;
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

	size_t MaterialData::ResourceCount() const
	{
		return m_Resources.size();
	}

	UUID* MaterialData::GetResourceUUIDPointer(size_t index)
	{
		return &m_Resources[index];
	}

	size_t MaterialData::GetResourcePropertyCount() const
	{
		return m_ResourcePropertyInfoIndices.size();
	}

	MaterialPropertyInfo* MaterialData::GetResourcePropertyInfo(size_t index)
	{
		size_t propertyIndex = m_ResourcePropertyInfoIndices[index];
		return &m_PropertyInfos[index];
	}

	size_t MaterialData::GetPropertyIndexFromResourceIndex(size_t index) const
	{
		return m_ResourcePropertyInfoIndices[index];
	}

	void MaterialData::ClearProperties()
	{
		m_PropertyBuffer.clear();
		m_PropertyInfos.clear();
		m_ResourcePropertyInfoIndices.clear();
		m_Resources.clear();
		m_HashToPropertyInfoIndex.clear();
		m_CurrentOffset = 0;
	}
}
