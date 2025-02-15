#include "EditorShaderData.h"
#include "EditorApplication.h"
#include "EditorPipelineManager.h"

#include <MaterialData.h>
#include <PipelineData.h>
#include <BinaryStream.h>

#include <ResourceType.h>

namespace Glory::Editor
{
	EditorShaderData::EditorShaderData(): m_UUID(0) {}
	EditorShaderData::EditorShaderData(UUID uuid) : m_UUID(uuid) {}
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
			pMaterial->AddResourceProperty(m_SamplerNames[i], m_SamplerNames[i], ResourceTypes::GetHash<TextureData>(), 0, textureType, 0);
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
		for (size_t i = 0; i < m_SamplerNames.size(); i++)
		{
			const TextureType textureType = EditorPipelineManager::ShaderNameToTextureType(m_SamplerNames[i]);
			pPipeline->AddResourceProperty(m_SamplerNames[i], m_SamplerNames[i], ResourceTypes::GetHash<TextureData>(), textureType);
		}

		for (size_t i = 0; i < m_PropertyInfos.size(); i++)
		{
			const EditorShaderData::PropertyInfo& info = m_PropertyInfos[i];
			ResourceTypes& types = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes();
			const BasicTypeData* pType = types.GetBasicTypeData(info.m_TypeHash);
			pPipeline->AddProperty(info.m_Name, info.m_Name, pType->m_TypeHash, pType->m_Size, 0);
		}
	}

	void EditorShaderData::Serialize(BinaryStream& container) const
	{
		container.Write(m_ShaderType).Write(m_ShaderData).
			Write(m_SamplerNames).Write(m_PropertyInfos.size());
		for (size_t i = 0; i < m_PropertyInfos.size(); ++i)
		{
			const PropertyInfo& prop = m_PropertyInfos[i];
			container.Write(prop.m_Name).Write(prop.m_TypeHash);
		}
		container.Write(m_Features);
	}

	void EditorShaderData::Deserialize(BinaryStream& container)
	{
		size_t numProperties;
		container.Read(m_ShaderType).Read(m_ShaderData).
			Read(m_SamplerNames).Read(numProperties);
		m_PropertyInfos.resize(numProperties);
		for (size_t i = 0; i < m_PropertyInfos.size(); ++i)
		{
			PropertyInfo& prop = m_PropertyInfos[i];
			container.Read(prop.m_Name).Read(prop.m_TypeHash);
		}
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