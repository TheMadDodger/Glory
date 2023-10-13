#include "MaterialData.h"
#include "AssetManager.h"
#include "BinaryStream.h"

#include <algorithm>

namespace Glory
{
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
		const auto it = std::find(m_pShaderFiles.begin(), m_pShaderFiles.end(), pShaderSourceData);
		if (it != m_pShaderFiles.end()) return false;
		m_pShaderFiles.push_back(pShaderSourceData);
		return true;
	}

	void MaterialData::AddProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t size, bool isResource, uint32_t flags)
	{
		const uint32_t hash = Reflect::Hash(displayName.data());
		const size_t index = m_PropertyInfos.size();
		size_t lastIndex = index - 1;
		m_PropertyInfos.emplace_back(MaterialPropertyInfo(displayName, shaderName, typeHash, size, m_CurrentOffset, flags));
		m_CurrentOffset = m_PropertyInfos[index].EndOffset();
		m_PropertyInfos[index].Reserve(m_PropertyBuffer);
		m_HashToPropertyInfoIndex[hash] = index;
	}

	void MaterialData::AddProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, UUID resourceUUID, uint32_t flags)
	{
		const uint32_t hash = Reflect::Hash(displayName.data());
		const size_t index = m_PropertyInfos.size();
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
		const uint32_t hash = Reflect::Hash(name.data());
		if (m_HashToPropertyInfoIndex.find(hash) == m_HashToPropertyInfoIndex.end()) return false;
		index = m_HashToPropertyInfoIndex.at(hash);
		return true;
	}

	size_t MaterialData::ResourceCount() const
	{
		return m_Resources.size();
	}

	AssetReference<TextureData>* MaterialData::GetResourceUUIDPointer(size_t index)
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

	void MaterialData::Serialize(BinaryStream& container) const
	{
		/* Write shader IDs */
		container.Write(m_pShaderFiles.size());
		for (size_t i = 0; i < m_pShaderFiles.size(); ++i)
		{
			container.Write(m_pShaderFiles[0]->GetUUID());
		}

		/* Write property infos */
		container.Write(m_PropertyInfos.size());
		for (size_t i = 0; i < m_PropertyInfos.size(); ++i)
		{
			const MaterialPropertyInfo& prop = m_PropertyInfos[i];
			container.Write(prop.TypeHash());
			container.Write(prop.ShaderName());
			container.Write(prop.DisplayName());
			container.Write(prop.Size());
			container.Write(prop.Offset());
			container.Write(prop.EndOffset());
			container.Write(prop.IsResource());
			container.Write(prop.Flags());
		}

		/* Write property buffer */
		container.Write(m_PropertyBuffer.size()).
			Write(m_PropertyBuffer.data(), m_PropertyBuffer.size());

		/* Write resources */
		container.Write(m_Resources.size());
		for (size_t i = 0; i < m_Resources.size(); ++i)
		{
			container.Write(m_Resources[i].AssetUUID());
		}
	}

	void MaterialData::Deserialize(BinaryStream& container) const
	{
	}

	void MaterialData::SetTexture(const std::string& name, TextureData* value)
	{
		size_t index;
		if (!GetPropertyInfoIndex(name, index)) return;
		EnableProperty(index);
		const MaterialPropertyInfo* pPropertyInfo = GetPropertyInfoAt(index);
		if (!pPropertyInfo->IsResource()) return;
		const size_t resourceIndex = pPropertyInfo->Offset();
		m_Resources[resourceIndex] = value ? value->GetUUID() : 0;
	}

	bool MaterialData::GetTexture(const std::string& name, TextureData** value)
	{
		size_t index;
		if (!GetPropertyInfoIndex(name, index)) return false;
		const MaterialPropertyInfo* pPropertyInfo = GetPropertyInfoAt(index);
		if (!pPropertyInfo->IsResource()) return false;
		const size_t resourceIndex = pPropertyInfo->Offset();
		*value = m_Resources[resourceIndex].Get();
		return *value;
	}

	void MaterialData::EnableProperty(size_t)
	{
	}

	std::vector<char>& MaterialData::GetPropertyBuffer(size_t)
	{
		return m_PropertyBuffer;
	}
}
