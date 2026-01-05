#include "MaterialData.h"
#include "AssetManager.h"
#include "BinaryStream.h"
#include "PipelineData.h"
#include "TextureData.h"
#include "PipelineManager.h"
#include "Engine.h"

#include <algorithm>

namespace Glory
{
	MaterialData::MaterialData()
		: m_Pipeline(0), m_CurrentOffset(0), m_TextureTypeIndices(TT_Count), m_TextureSetBits(0)
	{
		APPEND_TYPE(MaterialData);
	}

	MaterialData::~MaterialData()
	{
	}

	void MaterialData::AddProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t size, uint32_t flags)
	{
		const uint32_t hash = Reflect::Hash(displayName.data());
		if (m_HashToPropertyInfoIndex.find(hash) != m_HashToPropertyInfoIndex.end())
			return;

		const size_t index = m_PropertyInfos.size();
		const size_t lastIndex = index - 1;
		m_PropertyInfos.emplace_back(MaterialPropertyInfo(displayName, shaderName, typeHash, size, m_CurrentOffset, flags));
		m_CurrentOffset = m_PropertyInfos[index].EndOffset();
		m_PropertyInfos[index].Reserve(m_PropertyBuffer);
		m_HashToPropertyInfoIndex[hash] = index;
		void* pAddress = m_PropertyInfos[index].Address(m_PropertyBuffer);
		m_PropertyInfos[index].SetDefaultValue(pAddress);
	}

	void MaterialData::AddResourceProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, UUID resourceUUID, TextureType textureType, uint32_t flags)
	{
		const uint32_t hash = Reflect::Hash(displayName.data());
		if (m_HashToPropertyInfoIndex.find(hash) != m_HashToPropertyInfoIndex.end())
		{
			const size_t index = m_HashToPropertyInfoIndex.at(hash);
			const size_t resourceOffset = m_PropertyInfos[index].Offset();
			m_Resources[resourceOffset] = resourceUUID;
			if (resourceUUID)
				m_TextureSetBits |= 1u << index;
			else
				m_TextureSetBits &= ~(1u << index);
			return;
		}

		const size_t index = m_PropertyInfos.size();
		m_PropertyInfos.push_back(MaterialPropertyInfo(displayName, shaderName, typeHash, m_Resources.size(), textureType, flags));
		const size_t textureIndex = m_ResourcePropertyInfoIndices.size();
		m_ResourcePropertyInfoIndices.push_back(index);
		m_HashToPropertyInfoIndex[hash] = index;
		m_Resources.push_back(resourceUUID);
		if (resourceUUID)
			m_TextureSetBits |= 1u << index;
		else
			m_TextureSetBits &= ~(1u << index);
		m_TextureTypeIndices[size_t(textureType)].push_back(textureIndex);
	}

	void MaterialData::AddProperty(const MaterialPropertyInfo& other)
	{
		const uint32_t hash = Reflect::Hash(other.DisplayName().data());
		if (m_HashToPropertyInfoIndex.find(hash) != m_HashToPropertyInfoIndex.end())
			return;

		const size_t index = m_PropertyInfos.size();
		m_PropertyInfos.push_back(MaterialPropertyInfo(other));
		if (other.IsResource())
		{
			const size_t textureIndex = m_ResourcePropertyInfoIndices.size();
			m_ResourcePropertyInfoIndices.push_back(index);
			m_Resources.push_back(0);
			m_TextureSetBits &= ~(1u << index);
			m_TextureTypeIndices[size_t(other.m_TextureType)].push_back(textureIndex);
		}
		else
		{
			m_PropertyInfos[index].Reserve(m_PropertyBuffer);
			void* pAddress = m_PropertyInfos[index].Address(m_PropertyBuffer);
			m_PropertyInfos[index].SetDefaultValue(pAddress);
		}
		m_HashToPropertyInfoIndex[hash] = index;
	}

	void MaterialData::SetPipeline(PipelineData* pPipeline)
	{
		m_Pipeline = pPipeline->GetUUID();
	}

	void MaterialData::SetPipeline(UUID pipelineID)
	{
		m_Pipeline = pipelineID;
	}

	PipelineData* MaterialData::GetPipeline(const PipelineManager& pipelineManager) const
	{
		return m_Pipeline ? pipelineManager.GetPipelineData(m_Pipeline) : nullptr;
	}

	UUID MaterialData::GetPipelineID() const
	{
		return m_Pipeline;
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

	bool MaterialData::GetPropertyInfoIndex(const std::string& name, size_t& index) const
	{
		const uint32_t hash = Reflect::Hash(name.data());
		if (m_HashToPropertyInfoIndex.find(hash) == m_HashToPropertyInfoIndex.end()) return false;
		index = m_HashToPropertyInfoIndex.at(hash);
		return true;
	}

	bool MaterialData::GetPropertyInfoIndex(TextureType textureType, size_t texIndex, size_t& index) const
	{
		if (m_TextureTypeIndices[textureType].size() <= texIndex) return false;
		const size_t resourceIndex = m_TextureTypeIndices[textureType][texIndex];
		index = m_ResourcePropertyInfoIndices[resourceIndex];
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
		m_TextureSetBits = 0;

		for (size_t i = 0; i < m_TextureTypeIndices.size(); ++i)
		{
			m_TextureTypeIndices[i].clear();
		}
	}

	size_t MaterialData::TextureCount(TextureType textureType) const
	{
		return m_TextureTypeIndices[size_t(textureType)].size();
	}

	void MaterialData::Serialize(BinaryStream& container) const
	{
		/* Write pipeline ID */
		container.Write(m_Pipeline);

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
			container.Write(prop.GetTextureType());
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
		container.Write(m_TextureSetBits);
	}

	void MaterialData::Deserialize(BinaryStream& container)
	{
		/* Read pipeline ID */
		container.Read(m_Pipeline);

		/* Read property infos */
		size_t numProperties;
		container.Read(numProperties);
		m_PropertyInfos.resize(numProperties);
		for (size_t i = 0; i < m_PropertyInfos.size(); ++i)
		{
			MaterialPropertyInfo& prop = m_PropertyInfos[i];
			container.Read(prop.m_TypeHash);
			container.Read(prop.m_PropertyShaderName);
			container.Read(prop.m_PropertyDisplayName);
			container.Read(prop.m_Size);
			container.Read(prop.m_Offset);
			container.Read(prop.m_TextureType);
			container.Read(prop.m_Flags);

			const uint32_t hash = Reflect::Hash(prop.m_PropertyDisplayName.data());
			m_HashToPropertyInfoIndex[hash] = i;

			if (!prop.m_TextureType) continue;
			const size_t resourceIndex = m_ResourcePropertyInfoIndices.size();
			m_ResourcePropertyInfoIndices.push_back(i);
			m_TextureTypeIndices[size_t(prop.m_TextureType)].push_back(resourceIndex);
		}

		/* Read property buffer */
		size_t propertyBufferSize;
		container.Read(propertyBufferSize);
		m_PropertyBuffer.resize(propertyBufferSize);
		container.Read(m_PropertyBuffer.data(), propertyBufferSize);

		/* Read resources */
		size_t numResources;
		container.Read(numResources);
		m_Resources.resize(numResources);
		for (size_t i = 0; i < m_Resources.size(); ++i)
		{
			container.Read(*m_Resources[i].AssetUUIDMember());
		}
		container.Read(m_TextureSetBits);
	}

	void MaterialData::References(Engine* pEngine, std::vector<UUID>& references) const
	{
		if (m_Pipeline) references.push_back(m_Pipeline);
		for (auto& ref: m_Resources)
		{
			if (!ref.AssetUUID()) continue;
			references.push_back(ref.AssetUUID());
			TextureData* pTexture = pEngine->GetAssetManager().GetAssetImmediate<TextureData>(ref.AssetUUID());
			if (!pTexture) continue;
			pTexture->References(pEngine, references);
		}
	}

	void MaterialData::CopyProperties(void* dst)
	{
		auto& buffer = m_PropertyBuffer;
		std::memcpy(dst, buffer.data(), buffer.size());
	}

	size_t MaterialData::PropertyDataSize()
	{
		return m_PropertyBuffer.size();
	}

	MaterialData* MaterialData::CreateCopy() const
	{
		MaterialData* pCopy = new MaterialData();
		pCopy->m_Pipeline = m_Pipeline;
		for (auto& prop : m_PropertyInfos)
			pCopy->AddProperty(prop);

		pCopy->m_PropertyBuffer.resize(m_PropertyBuffer.size());
		std::memcpy(pCopy->m_PropertyBuffer.data(), m_PropertyBuffer.data(), m_PropertyBuffer.size());
		std::memcpy(pCopy->m_Resources.data(), m_Resources.data(), m_Resources.size()*sizeof(UUID));
		return pCopy;
	}

	uint32_t MaterialData::TextureSetBits() const
	{
		return m_TextureSetBits;
	}

	void* MaterialData::Address(size_t index)
	{
		return m_PropertyInfos[index].Address(m_PropertyBuffer);
	}

	void MaterialData::SetTexture(const std::string& name, TextureData* value)
	{
		const UUID uuid = value ? value->GetUUID() : 0;
		SetTexture(name, uuid);
	}

	void MaterialData::SetTexture(const std::string& name, UUID uuid)
	{
		size_t index;
		if (!GetPropertyInfoIndex(name, index)) return;
		const MaterialPropertyInfo* pPropertyInfo = GetPropertyInfoAt(index);
		if (!pPropertyInfo->IsResource()) return;
		const size_t resourceIndex = pPropertyInfo->Offset();
		m_Resources[resourceIndex] = uuid;

		if (uuid)
			m_TextureSetBits |= 1u << index;
		else
			m_TextureSetBits &= ~(1u << index);
	}

	void MaterialData::SetTexture(TextureType textureType, size_t texIndex, UUID uuid)
	{
		size_t index;
		if (!GetPropertyInfoIndex(textureType, texIndex, index)) return;
		const MaterialPropertyInfo* pPropertyInfo = GetPropertyInfoAt(index);
		if (!pPropertyInfo->IsResource()) return;
		const size_t resourceIndex = pPropertyInfo->Offset();
		m_Resources[resourceIndex] = uuid;

		if (uuid)
			m_TextureSetBits |= 1u << index;
		else
			m_TextureSetBits &= ~(1u << index);
	}

	bool MaterialData::GetTexture(const std::string& name, TextureData** value, AssetManager* pManager)
	{
		size_t index;
		if (!GetPropertyInfoIndex(name, index)) return false;
		const MaterialPropertyInfo* pPropertyInfo = GetPropertyInfoAt(index);
		if (!pPropertyInfo->IsResource()) return false;
		const size_t resourceIndex = pPropertyInfo->Offset();
		*value = m_Resources[resourceIndex].Get(pManager);
		return true;
	}
}
