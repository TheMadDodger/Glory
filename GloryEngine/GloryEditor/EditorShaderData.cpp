#include "EditorShaderData.h"
#include <ResourceType.h>

namespace Glory::Editor
{
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

	void EditorShaderData::LoadIntoMaterial(MaterialData* pMaterial)
	{
		for (size_t i = 0; i < m_SamplerNames.size(); i++)
		{
			pMaterial->AddProperty(m_SamplerNames[i], m_SamplerNames[i], ResourceType::GetHash<ImageData>(), nullptr);
		}

		if (pMaterial->GetCurrentBufferOffset() > 0) return; // Already added from other shader
		for (size_t i = 0; i < m_PropertyInfos.size(); i++)
		{
			EditorShaderData::PropertyInfo info = m_PropertyInfos[i];
			const BasicTypeData* pType = ResourceType::GetBasicTypeData(info.m_TypeHash);
			pMaterial->AddProperty(info.m_Name, info.m_Name, pType->m_TypeHash, pType->m_Size, 0);
		}
	}

	EditorShaderData::PropertyInfo::PropertyInfo(const std::string& name, size_t typeHash) : m_Name(name), m_TypeHash(typeHash)
	{
	}
}