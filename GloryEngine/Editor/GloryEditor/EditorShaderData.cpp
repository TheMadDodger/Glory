#include "EditorShaderData.h"
#include "EditorApplication.h"
#include "EditorPipelineManager.h"

#include <MaterialData.h>
#include <PipelineData.h>
#include <CubemapData.h>
#include <BinaryStream.h>

#include <ResourceType.h>

namespace Glory::Editor
{
	const uint32_t ImageTypeToHash[size_t(ImageType::IT_Count)] = {
		0, /* Undefined */
		0, /* 1D currently not supporter */
		ResourceTypes::GetHash<TextureData>(), /* 2D */
		0, /* 3D currently not supporter */
		ResourceTypes::GetHash<CubemapData>(), /* Cubemap */
		0, /* 1D array currently not supporter */
		0, /* 2D array currently not supporter */
		0, /* Cube array currently not supporter */
	};

	EditorShaderData::EditorShaderData(): m_UUID(0), m_ShaderType(ShaderType::ST_Unknown) {}
	EditorShaderData::EditorShaderData(UUID uuid) : m_UUID(uuid), m_ShaderType(ShaderType::ST_Unknown) {}
	EditorShaderData::~EditorShaderData() {}

	std::vector<uint32_t>::const_iterator EditorShaderData::Begin()
	{
		return m_ShaderData.begin();
	}

	std::vector<uint32_t>::const_iterator EditorShaderData::End()
	{
		return m_ShaderData.end();
	}

	const uint32_t* EditorShaderData::Data() const
	{
		return m_ShaderData.data();
	}

	size_t EditorShaderData::Size() const
	{
		return m_ShaderData.size();
	}

	UUID EditorShaderData::GetUUID() const
	{
		return m_UUID;
	}

	void EditorShaderData::LoadIntoMaterial(MaterialData* pMaterial) const
	{
		MaterialManager& manager = EditorApplication::GetInstance()->GetEngine()->GetMaterialManager();

		for (size_t i = 0; i < m_SamplerNames.size(); ++i)
		{
			const TextureType textureType = EditorPipelineManager::ShaderNameToTextureType(m_SamplerNames[i]);
			const ImageType imageType = m_SamplerTypes[i];
			const uint32_t hash = ImageTypeToHash[size_t(imageType)];
			pMaterial->AddResourceProperty(m_SamplerNames[i], m_SamplerNames[i], hash, 0, textureType, 0);
		}

		if (pMaterial->GetCurrentBufferOffset(manager) > 0) return; // Already added from other shader
		for (size_t i = 0; i < m_PropertyInfos.size(); ++i)
		{
			const EditorShaderData::PropertyInfo& info = m_PropertyInfos[i];
			ResourceTypes& types = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes();
			const BasicTypeData* pType = types.GetBasicTypeData(info.m_TypeHash);
			pMaterial->AddProperty(info.m_Name, info.m_Name, pType->m_TypeHash, pType->m_Size, 0);
		}
	}

	void EditorShaderData::LoadIntoPipeline(PipelineData* pPipeline) const
	{
		for (size_t i = 0; i < m_SamplerNames.size(); ++i)
		{
			const TextureType textureType = EditorPipelineManager::ShaderNameToTextureType(m_SamplerNames[i]);
			const ImageType imageType = m_SamplerTypes[i];
			const uint32_t hash = ImageTypeToHash[size_t(imageType)];
			pPipeline->AddResourceProperty(m_SamplerNames[i], m_SamplerNames[i], hash, textureType);
		}

		for (size_t i = 0; i < m_PropertyInfos.size(); ++i)
		{
			const EditorShaderData::PropertyInfo& info = m_PropertyInfos[i];
			ResourceTypes& types = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes();
			const BasicTypeData* pType = types.GetBasicTypeData(info.m_TypeHash);
			pPipeline->AddProperty(info.m_Name, info.m_Name, pType->m_TypeHash, pType->m_Size, 0);
		}

		for (size_t i = 0; i < m_UniformBuffers.size(); ++i)
		{
			const std::string& uniformBuffer = m_UniformBuffers[i];
			pPipeline->AddUniformBuffer(uniformBuffer, m_ShaderType);
		}

		for (size_t i = 0; i < m_StorageBuffers.size(); ++i)
		{
			const std::string& storageBuffer = m_StorageBuffers[i];
			pPipeline->AddStorageBuffer(storageBuffer, m_ShaderType);
		}
	}

	void EditorShaderData::Serialize(BinaryStream& container) const
	{
		container.Write(m_ShaderType).Write(m_ShaderData).
			Write(m_SamplerNames).Write(m_SamplerTypes).Write(m_PropertyInfos.size());
		for (size_t i = 0; i < m_PropertyInfos.size(); ++i)
		{
			const PropertyInfo& prop = m_PropertyInfos[i];
			container.Write(prop.m_Name).Write(prop.m_TypeHash);
		}
		container.Write(m_UniformBuffers);
		container.Write(m_StorageBuffers);
		container.Write(m_Features);
	}

	void EditorShaderData::Deserialize(BinaryStream& container)
	{
		size_t numProperties;
		container.Read(m_ShaderType).Read(m_ShaderData).
			Read(m_SamplerNames).Read(m_SamplerTypes).Read(numProperties);
		m_PropertyInfos.resize(numProperties);
		for (size_t i = 0; i < m_PropertyInfos.size(); ++i)
		{
			PropertyInfo& prop = m_PropertyInfos[i];
			container.Read(prop.m_Name).Read(prop.m_TypeHash);
		}
		container.Read(m_UniformBuffers);
		container.Read(m_StorageBuffers);
		container.Read(m_Features);
	}

	EditorShaderData::PropertyInfo::PropertyInfo():
		m_Name(""), m_TypeHash(0)
	{
	}

	EditorShaderData::PropertyInfo::PropertyInfo(const std::string& name, uint32_t typeHash):
		m_Name(name), m_TypeHash(typeHash)
	{
	}
}