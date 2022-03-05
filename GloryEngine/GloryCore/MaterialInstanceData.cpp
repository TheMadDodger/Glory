#include "MaterialInstanceData.h"
#include "ResourceType.h"
#include <algorithm>

namespace Glory
{
	MaterialInstanceData::MaterialInstanceData(MaterialData* pBaseMaterial) : m_pBaseMaterial(pBaseMaterial)
	{
		APPEND_TYPE(MaterialInstanceData);

		if (!pBaseMaterial) return;

		pBaseMaterial->CopyProperties(m_Properties);

		m_PropertyOverridesEnable.resize(m_Properties.size(), false);

		for (size_t i = 0; i < m_Properties.size(); i++)
		{
			MaterialPropertyData* pProperty = &m_Properties[i];
			size_t hash = m_Hasher(pProperty->Name());
			m_HashToPropertyIndex.emplace(hash, i);
		}
	}

	MaterialInstanceData::~MaterialInstanceData()
	{
		m_pBaseMaterial = nullptr;
	}

	size_t MaterialInstanceData::ShaderCount() const
	{
		return m_pBaseMaterial->ShaderCount();
	}

	ShaderSourceData* MaterialInstanceData::GetShaderAt(size_t index) const
	{
		return m_pBaseMaterial->GetShaderAt(index);
	}

	const ShaderType& MaterialInstanceData::GetShaderTypeAt(size_t index) const
	{
		return m_pBaseMaterial->GetShaderTypeAt(index);
	}

	MaterialData* MaterialInstanceData::GetBaseMaterial() const
	{
		return m_pBaseMaterial;
	}

	const UUID& MaterialInstanceData::GetGPUUUID() const
	{
		return m_pBaseMaterial->GetUUID();
	}

	void MaterialInstanceData::CopyProperties(std::vector<MaterialPropertyData>& destination)
	{
		std::unique_lock lock(m_PropertiesAccessMutex);
		destination.clear();

		for (size_t i = 0; i < m_Properties.size(); i++)
		{
			if (m_PropertyOverridesEnable[i])
			{
				destination.push_back(MaterialPropertyData(m_Properties[i]));
				continue;
			}
			destination.push_back(m_pBaseMaterial->CopyPropertyAt(i));
		}
		lock.unlock();
	}

	void MaterialInstanceData::PasteProperties(const std::vector<MaterialPropertyData>& destination)
	{
		std::unique_lock lock(m_PropertiesAccessMutex);
		m_Properties.clear();
		std::for_each(destination.begin(), destination.end(), [&](const MaterialPropertyData& propertyData)
		{
			m_Properties.push_back(MaterialPropertyData(propertyData));
		});
		lock.unlock();
	}

	void MaterialInstanceData::CopyOverrideStates(std::vector<bool>& destination)
	{
		std::unique_lock lock(m_PropertiesAccessMutex);
		if (m_PropertyOverridesEnable.size() != destination.size()) destination.resize(m_PropertyOverridesEnable.size());
		destination.assign(m_PropertyOverridesEnable.begin(), m_PropertyOverridesEnable.end());
		lock.unlock();
	}

	void MaterialInstanceData::PasteOverrideStates(std::vector<bool>& destination)
	{
		std::unique_lock lock(m_PropertiesAccessMutex);
		if (m_PropertyOverridesEnable.size() != destination.size()) m_PropertyOverridesEnable.resize(destination.size());
		m_PropertyOverridesEnable.assign(destination.begin(), destination.end());
		lock.unlock();
	}
}
