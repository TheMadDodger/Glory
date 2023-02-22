#include "EditorShaderData.h"
#include <ResourceType.h>

namespace Glory::Editor
{
	GLORY_EDITOR_API EditorShaderData::EditorShaderData(UUID uuid) : m_UUID(uuid) {}
	GLORY_EDITOR_API EditorShaderData::~EditorShaderData() {}

	GLORY_EDITOR_API std::vector<uint32_t>::const_iterator EditorShaderData::Begin()
	{
		return m_ShaderData.begin();
	}

	GLORY_EDITOR_API std::vector<uint32_t>::const_iterator EditorShaderData::End()
	{
		return m_ShaderData.end();
	}

	GLORY_EDITOR_API const uint32_t* EditorShaderData::Data() const
	{
		return m_ShaderData.data();
	}

	GLORY_EDITOR_API size_t EditorShaderData::Size() const
	{
		return m_ShaderData.size();
	}

	GLORY_EDITOR_API UUID EditorShaderData::GetUUID() const
	{
		return m_UUID;
	}

	GLORY_EDITOR_API void EditorShaderData::LoadIntoMaterial(MaterialData* pMaterial)
	{
		for (size_t i = 0; i < m_SamplerNames.size(); i++)
		{
			pMaterial->AddProperty(m_SamplerNames[i], m_SamplerNames[i], ResourceType::GetHash<ImageData>(), 0);
		}

		if (pMaterial->GetCurrentBufferOffset() > 0) return; // Already added from other shader
		for (size_t i = 0; i < m_PropertyInfos.size(); i++)
		{
			EditorShaderData::PropertyInfo info = m_PropertyInfos[i];
			const BasicTypeData* pType = ResourceType::GetBasicTypeData(info.m_TypeHash);
			pMaterial->AddProperty(info.m_Name, info.m_Name, pType->m_TypeHash, pType->m_Size, 0);
		}
	}

	GLORY_EDITOR_API EditorShaderData::PropertyInfo::PropertyInfo(const std::string& name, uint32_t typeHash) : m_Name(name), m_TypeHash(typeHash)
	{
	}
}