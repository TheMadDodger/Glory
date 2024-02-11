#include "MaterialData.h"
#include "AssetManager.h"
#include "BinaryStream.h"
#include "ShaderManager.h"

#include <algorithm>

namespace Glory
{
	MaterialData::MaterialData()
		: m_CurrentOffset(0)
	{
		APPEND_TYPE(MaterialData);
	}

	MaterialData::~MaterialData()
	{
		m_Shaders.clear();
	}

	size_t MaterialData::ShaderCount(const MaterialManager&) const
	{
		return m_Shaders.size();
	}

	ShaderType MaterialData::GetShaderTypeAt(const MaterialManager& materialManager, ShaderManager& manager, size_t index) const
	{
		const UUID shaderID = GetShaderIDAt(materialManager, index);
		return manager.GetShaderType(shaderID);
	}

	FileData* MaterialData::GetShaderAt(const MaterialManager& materialManager, ShaderManager& manager, size_t index) const
	{
		const UUID shaderID = GetShaderIDAt(materialManager, index);
		return manager.GetCompiledShaderFile(shaderID);
	}

	UUID MaterialData::GetShaderIDAt(const MaterialManager&, size_t index) const
	{
		return m_Shaders[index];
	}

	void MaterialData::RemoveShaderAt(size_t index)
	{
		m_Shaders.erase(m_Shaders.begin() + index);
	}

	void MaterialData::RemoveAllShaders()
	{
		m_Shaders.clear();
	}

	bool MaterialData::AddShader(UUID shaderID)
	{
		const auto it = std::find(m_Shaders.begin(), m_Shaders.end(), shaderID);
		if (it != m_Shaders.end()) return false;
		m_Shaders.push_back(shaderID);
		return true;
	}

	void MaterialData::AddProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t size, bool isResource, uint32_t flags)
	{
		const uint32_t hash = Reflect::Hash(displayName.data());
		if (m_HashToPropertyInfoIndex.find(hash) != m_HashToPropertyInfoIndex.end())
			return;

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
		if (m_HashToPropertyInfoIndex.find(hash) != m_HashToPropertyInfoIndex.end())
		{
			const size_t index = m_HashToPropertyInfoIndex.at(hash);
			const size_t resourceOffset = m_PropertyInfos[index].Offset();
			m_Resources[resourceOffset] = resourceUUID;
			return;
		}

		const size_t index = m_PropertyInfos.size();
		m_PropertyInfos.push_back(MaterialPropertyInfo(displayName, shaderName, typeHash, m_Resources.size(), flags));
		m_ResourcePropertyInfoIndices.push_back(index);
		m_HashToPropertyInfoIndex[hash] = index;
		m_Resources.push_back(resourceUUID);
	}

	size_t MaterialData::PropertyInfoCount(const MaterialManager&) const
	{
		return m_PropertyInfos.size();
	}

	MaterialPropertyInfo* MaterialData::GetPropertyInfoAt(const MaterialManager& , size_t index)
	{
		return &m_PropertyInfos[index];
	}

	size_t MaterialData::GetCurrentBufferOffset(const MaterialManager&) const
	{
		return m_CurrentOffset;
	}

	std::vector<char>& MaterialData::GetBufferReference(const MaterialManager&)
	{
		return m_PropertyBuffer;
	}

	std::vector<char>& MaterialData::GetFinalBufferReference(MaterialManager&)
	{
		return m_PropertyBuffer;
	}

	bool MaterialData::GetPropertyInfoIndex(const MaterialManager&, const std::string& name, size_t& index) const
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

	AssetReference<TextureData>* MaterialData::GetResourceUUIDPointer(MaterialManager&, size_t index)
	{
		return &m_Resources[index];
	}

	size_t MaterialData::GetResourcePropertyCount(MaterialManager&) const
	{
		return m_ResourcePropertyInfoIndices.size();
	}

	MaterialPropertyInfo* MaterialData::GetResourcePropertyInfo(MaterialManager&, size_t index)
	{
		size_t propertyIndex = m_ResourcePropertyInfoIndices[index];
		return &m_PropertyInfos[index];
	}

	size_t MaterialData::GetPropertyIndexFromResourceIndex(MaterialManager&, size_t index) const
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
		container.Write(m_Shaders.size());
		for (size_t i = 0; i < m_Shaders.size(); ++i)
		{
			container.Write(m_Shaders[i]);
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

	bool MaterialData::HasShader(const UUID shaderID) const
	{
		return std::find(m_Shaders.begin(), m_Shaders.end(), shaderID) != m_Shaders.end();
	}

	void MaterialData::SetTexture(MaterialManager& materialManager, const std::string& name, TextureData* value)
	{
		const UUID uuid = value ? value->GetUUID() : 0;
		SetTexture(materialManager, name, uuid);
	}

	void MaterialData::SetTexture(MaterialManager& materialManager, const std::string& name, UUID uuid)
	{
		size_t index;
		if (!GetPropertyInfoIndex(materialManager, name, index)) return;
		EnableProperty(index);
		const MaterialPropertyInfo* pPropertyInfo = GetPropertyInfoAt(materialManager, index);
		if (!pPropertyInfo->IsResource()) return;
		const size_t resourceIndex = pPropertyInfo->Offset();
		m_Resources[resourceIndex] = uuid;
	}

	bool MaterialData::GetTexture(MaterialManager& materialManager, const std::string& name, TextureData** value, AssetManager* pManager)
	{
		size_t index;
		if (!GetPropertyInfoIndex(materialManager, name, index)) return false;
		const MaterialPropertyInfo* pPropertyInfo = GetPropertyInfoAt(materialManager, index);
		if (!pPropertyInfo->IsResource()) return false;
		const size_t resourceIndex = pPropertyInfo->Offset();
		*value = m_Resources[resourceIndex].Get(pManager);
		return *value;
	}

	void MaterialData::EnableProperty(size_t)
	{
	}

	std::vector<char>& MaterialData::GetPropertyBuffer(MaterialManager&, size_t)
	{
		return m_PropertyBuffer;
	}
}
