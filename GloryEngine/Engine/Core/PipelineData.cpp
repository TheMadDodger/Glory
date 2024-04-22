#include "PipelineData.h"

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

	void PipelineData::AddProperty(const std::string& displayName, const std::string& shaderName, uint32_t typeHash)
	{
		const uint32_t hash = Reflect::Hash(displayName.data());
		if (m_HashToPropertyInfoIndex.find(hash) != m_HashToPropertyInfoIndex.end())
		{
			const size_t index = m_HashToPropertyInfoIndex.at(hash);
			const size_t resourceOffset = m_PropertyInfos[index].Offset();
			return;
		}

		const size_t index = m_PropertyInfos.size();
		m_PropertyInfos.push_back(MaterialPropertyInfo(displayName, shaderName, typeHash, m_NumResources, 0));
		m_ResourcePropertyInfoIndices.push_back(index);
		m_HashToPropertyInfoIndex[hash] = index;
		++m_NumResources;
	}

	size_t PipelineData::PropertyInfoCount(const MaterialManager&) const
	{
		return m_PropertyInfos.size();
	}

	MaterialPropertyInfo* PipelineData::GetPropertyInfoAt(const MaterialManager&, size_t index)
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
	}

	void PipelineData::Deserialize(BinaryStream& container)
	{
	}
}
