#include "PipelineData.h"
#include "MaterialData.h"
#include "ShaderManager.h"
#include "BinaryStream.h"

namespace Glory
{
	PipelineData::PipelineData()
	{
		APPEND_TYPE(PipelineData);
	}

	PipelineData::~PipelineData()
	{
	}

	size_t PipelineData::ShaderCount() const
	{
		return m_Shaders.size();
	}

	PipelineType PipelineData::Type() const
	{
		return m_Type;
	}

	UUID PipelineData::ShaderID(size_t index) const
	{
		return m_Shaders[index];
	}

	FileData* PipelineData::Shader(const ShaderManager& manager, size_t index) const
	{
		return manager.GetCompiledShaderFile(ShaderID(index));
	}

	ShaderType PipelineData::GetShaderType(const ShaderManager& manager, size_t index) const
	{
		return manager.GetShaderType(ShaderID(index));
	}

	void PipelineData::SetPipelineType(PipelineType type)
	{
		m_Type = type;
	}

	void PipelineData::AddShader(UUID shaderID)
	{
		m_Shaders.push_back(shaderID);
	}

	void PipelineData::RemoveShaderAt(size_t index)
	{
		m_Shaders.erase(m_Shaders.begin() + index);
	}

	void PipelineData::AddProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, size_t size, uint32_t flags)
	{
		const uint32_t hash = Reflect::Hash(displayName.data());
		if (m_HashToPropertyInfoIndex.find(hash) != m_HashToPropertyInfoIndex.end())
			return;

		const size_t index = m_PropertyInfos.size();
		size_t lastIndex = index - 1;
		m_PropertyInfos.emplace_back(MaterialPropertyInfo(displayName, shaderName, typeHash, size, m_CurrentOffset, flags));
		m_CurrentOffset = m_PropertyInfos[index].EndOffset();
		m_HashToPropertyInfoIndex[hash] = index;
	}

	void PipelineData::AddResourceProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash, TextureType textureType)
	{
		const uint32_t hash = Reflect::Hash(displayName.data());
		if (m_HashToPropertyInfoIndex.find(hash) != m_HashToPropertyInfoIndex.end())
		{
			const size_t index = m_HashToPropertyInfoIndex.at(hash);
			const size_t resourceOffset = m_PropertyInfos[index].Offset();
			return;
		}

		const size_t index = m_PropertyInfos.size();
		m_PropertyInfos.push_back(MaterialPropertyInfo(displayName, shaderName, typeHash, m_NumResources, textureType, 0));
		m_ResourcePropertyInfoIndices.push_back(index);
		m_HashToPropertyInfoIndex[hash] = index;
		++m_NumResources;
	}

	size_t PipelineData::PropertyInfoCount() const
	{
		return m_PropertyInfos.size();
	}

	MaterialPropertyInfo* PipelineData::GetPropertyInfoAt(size_t index)
	{
		return &m_PropertyInfos[index];
	}

	void PipelineData::ClearProperties()
	{
		m_PropertyInfos.clear();
		m_ResourcePropertyInfoIndices.clear();
		m_NumResources = 0;
		m_HashToPropertyInfoIndex.clear();
		m_CurrentOffset = 0;
	}

	bool PipelineData::HasShader(const UUID shaderID) const
	{
		return std::find(m_Shaders.begin(), m_Shaders.end(), shaderID) != m_Shaders.end();
	}

	void PipelineData::RemoveAllShaders()
	{
		m_Shaders.clear();
	}

	void PipelineData::Serialize(BinaryStream& container) const
	{
		/* Write pipeline type */
		container.Write(m_Type);

		/* Write shader IDs */
		container.Write(m_Shaders.size());
		for (size_t i = 0; i < m_Shaders.size(); ++i)
			container.Write(m_Shaders[i]);

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
			container.Write(prop.IsResource());
			container.Write(prop.Flags());
		}
	}

	void PipelineData::Deserialize(BinaryStream& container)
	{
		/* Read pipeline type */
		container.Read(m_Type);

		/* Read shader IDs */
		size_t numShaders;
		container.Read(numShaders);
		m_Shaders.resize(numShaders);
		for (size_t i = 0; i < numShaders; ++i)
			container.Read(m_Shaders[i]);

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

			if (!prop.m_TextureType) continue;
			m_ResourcePropertyInfoIndices.push_back(i);
		}
	}

	void PipelineData::LoadIntoMaterial(MaterialData* pMaterial) const
	{
		pMaterial->ClearProperties();
		for (size_t i = 0; i < m_PropertyInfos.size(); ++i)
			pMaterial->AddProperty(m_PropertyInfos[i]);
	}

	bool PipelineData::UsesTextures() const
	{
		for (size_t i = 0; i < m_PropertyInfos.size(); ++i)
		{
			if (!m_PropertyInfos[i].m_TextureType) continue;
			return true;
		}
		return false;
	}
}
