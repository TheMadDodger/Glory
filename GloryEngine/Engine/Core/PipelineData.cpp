#include "PipelineData.h"
#include "MaterialData.h"
#include "PipelineManager.h"
#include "BinaryStream.h"

namespace Glory
{
	PipelineData::PipelineData() : m_TotalPropertiesByteSize(0)
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

	const FileData* PipelineData::Shader(const PipelineManager& manager, size_t index) const
	{
		auto& shaders = manager.GetPipelineCompiledShaders(GetUUID());
		if (index >= shaders.size()) return nullptr;
		return &shaders[index];
	}

	ShaderType PipelineData::GetShaderType(const PipelineManager& manager, size_t index) const
	{
		auto& types = manager.GetPipelineShaderTypes(GetUUID());
		if (index >= types.size()) return ShaderType::ST_Unknown;
		return types[index];
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
		m_TotalPropertiesByteSize += size;
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
		m_TotalPropertiesByteSize = 0;
	}

	void PipelineData::AddUniformBuffer(const std::string& name, ShaderType shaderType)
	{
		auto iter = std::find_if(m_UniformBuffers.begin(), m_UniformBuffers.end(), [&name](const ShaderBufferInfo& bufferInfo) { return bufferInfo.Name == name; });
		if (iter == m_UniformBuffers.end())
		{
			m_UniformBuffers.emplace_back(ShaderBufferInfo{ name, ShaderTypeToFlag[size_t(shaderType)] });
			return;
		}
		iter->ShaderFlags = ShaderTypeFlag(iter->ShaderFlags | ShaderTypeToFlag[size_t(shaderType)]);
	}

	void PipelineData::AddStorageBuffer(const std::string& name, ShaderType shaderType)
	{
		auto iter = std::find_if(m_StorageBuffers.begin(), m_StorageBuffers.end(), [&name](const ShaderBufferInfo& bufferInfo) { return bufferInfo.Name == name; });
		if (iter == m_StorageBuffers.end())
		{
			m_StorageBuffers.emplace_back(ShaderBufferInfo{ name, ShaderTypeToFlag[size_t(shaderType)] });
			return;
		}
		iter->ShaderFlags = ShaderTypeFlag(iter->ShaderFlags | ShaderTypeToFlag[size_t(shaderType)]);
	}

	size_t PipelineData::UniformBufferCount() const
	{
		return m_UniformBuffers.size();
	}

	size_t PipelineData::StorageBufferCount() const
	{
		return m_StorageBuffers.size();
	}

	const ShaderBufferInfo& PipelineData::UniformBuffer(size_t index) const
	{
		return m_UniformBuffers[index];
	}

	const ShaderBufferInfo& PipelineData::StorageBuffer(size_t index) const
	{
		return m_StorageBuffers[index];
	}

	size_t PipelineData::ResourcePropertyCount() const
	{
		return m_ResourcePropertyInfoIndices.size();
	}

	MaterialPropertyInfo* PipelineData::ResourcePropertyInfo(size_t resourceIndex)
	{
		const size_t propertyIndex = m_ResourcePropertyInfoIndices[resourceIndex];
		return &m_PropertyInfos[propertyIndex];
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
			container.Write(prop.GetTextureType());
			container.Write(prop.Flags());
		}

		/* Write buffer infos */
		container.Write(m_UniformBuffers.size());
		for (size_t i = 0; i < m_UniformBuffers.size(); ++i)
		{
			container.Write(m_UniformBuffers[i].Name).
				Write(uint8_t(m_UniformBuffers[i].ShaderFlags));
		}
		container.Write(m_StorageBuffers.size());
		for (size_t i = 0; i < m_StorageBuffers.size(); ++i)
		{
			container.Write(m_StorageBuffers[i].Name).
				Write(uint8_t(m_StorageBuffers[i].ShaderFlags));
		}

		container.Write(m_TotalPropertiesByteSize);

		/* Write settings */
		container.Write(m_CullFace).Write(m_PrimitiveType).Write(m_SettingsToggles);
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

		/* Read buffer infos */
		size_t numUniformBuffers;
		container.Read(numUniformBuffers);
		m_UniformBuffers.resize(numUniformBuffers);
		for (size_t i = 0; i < m_UniformBuffers.size(); ++i)
		{
			container.Read(m_UniformBuffers[i].Name).
				Read(reinterpret_cast<uint8_t&>(m_UniformBuffers[i].ShaderFlags));
		}
		size_t numStorageBuffers;
		container.Read(numStorageBuffers);
		m_StorageBuffers.resize(numStorageBuffers);
		for (size_t i = 0; i < m_StorageBuffers.size(); ++i)
		{
			container.Read(m_StorageBuffers[i].Name).
				Read(reinterpret_cast<uint8_t&>(m_StorageBuffers[i].ShaderFlags));
		}

		container.Read(m_TotalPropertiesByteSize);

		/* Read settings */
		container.Read(m_CullFace).Read(m_PrimitiveType).Read(m_SettingsToggles);
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

	void PipelineData::AddFeature(std::string_view feature, bool isOn)
	{
		const size_t index = FeatureIndex(feature);
		if (index != m_Features.size())
		{
			m_FeaturesEnabled.Set(index, isOn);
			return;
		}

		m_Features.push_back(std::string(feature));
		m_FeaturesEnabled.Reserve(m_Features.size());
		m_FeaturesEnabled.Set(index, isOn);
	}

	size_t PipelineData::FeatureIndex(std::string_view feature) const
	{
		for (size_t i = 0; i < m_Features.size(); ++i)
		{
			if (m_Features[i] != feature) continue;
			return i;
		}
		return m_Features.size();
	}

	size_t PipelineData::FeatureCount() const
	{
		return m_Features.size();
	}

	std::string_view PipelineData::FeatureName(size_t index) const
	{
		return m_Features[index];
	}

	bool PipelineData::FeatureEnabled(size_t index)
	{
		if (index >= m_Features.size()) return true;
		return m_FeaturesEnabled.IsSet(index);
	}

	void PipelineData::SetFeatureEnabled(size_t index, bool enabled)
	{
		m_FeaturesEnabled.Set(index, enabled);
	}

	void PipelineData::ClearFeatures()
	{
		m_Features.clear();
		m_FeaturesEnabled.Clear();
	}
	size_t PipelineData::TotalPropertiesByteSize() const
	{
		return m_TotalPropertiesByteSize;
	}

	void PipelineData::SetDepthTestEnabled(bool enable)
	{
		m_SettingsToggles.Set(SettingBitsIndices::DepthTestEnable, enable);
	}
	const bool PipelineData::DepthTestEnabled() const
	{
		return m_SettingsToggles.IsSet(SettingBitsIndices::DepthTestEnable);
	}

	void PipelineData::SetDepthWriteEnabled(bool enable)
	{
		m_SettingsToggles.Set(SettingBitsIndices::DepthWriteEnable, enable);
	}

	const bool PipelineData::DepthWriteEnabled() const
	{
		return m_SettingsToggles.IsSet(SettingBitsIndices::DepthWriteEnable);
	}
}
