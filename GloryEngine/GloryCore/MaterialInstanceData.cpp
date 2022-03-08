#include "MaterialInstanceData.h"
#include "ResourceType.h"
#include <algorithm>

namespace Glory
{
	MaterialInstanceData::MaterialInstanceData(MaterialData* pBaseMaterial) : m_pBaseMaterial(pBaseMaterial)
	{
		APPEND_TYPE(MaterialInstanceData);

		if (!pBaseMaterial) return;

		m_PropertyOverridesEnable.resize(pBaseMaterial->PropertyInfoCount(), false);
		m_PropertyBuffer.resize(pBaseMaterial->GetBufferReference().size());
		m_pResources.resize(pBaseMaterial->ResourceCount());
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

	size_t MaterialInstanceData::PropertyInfoCount() const
	{
		return m_pBaseMaterial->PropertyInfoCount();
	}

	MaterialPropertyInfo* MaterialInstanceData::GetPropertyInfoAt(size_t index)
	{
		return m_pBaseMaterial->GetPropertyInfoAt(index);
	}

	size_t MaterialInstanceData::GetCurrentBufferOffset() const
	{
		return m_pBaseMaterial->GetCurrentBufferOffset();
	}

	std::vector<char>& MaterialInstanceData::GetBufferReference()
	{
		std::vector<char>& baseBuffer = m_pBaseMaterial->GetBufferReference();
		m_PropertyBuffer.resize(baseBuffer.size());
		for (size_t i = 0; i < m_pBaseMaterial->PropertyInfoCount(); i++)
		{
			MaterialPropertyInfo* propertyInfo = m_pBaseMaterial->GetPropertyInfoAt(i);
			if (propertyInfo->IsResource()) continue;
			if (m_PropertyOverridesEnable[i]) continue;
			memcpy(&m_PropertyBuffer[propertyInfo->Offset()], &baseBuffer[propertyInfo->Offset()], propertyInfo->Size());
		}
		return m_PropertyBuffer;
	}

	bool MaterialInstanceData::GetPropertyInfoIndex(const std::string& name, size_t& index) const
	{
		return m_pBaseMaterial->GetPropertyInfoIndex(name, index);
	}

	Resource** MaterialInstanceData::GetResourcePointer(size_t index)
	{
		size_t propertyIndex = GetPropertyIndexFromResourceIndex(index);
		return m_PropertyOverridesEnable[propertyIndex] ? &m_pResources[index] : m_pBaseMaterial->GetResourcePointer(index);
	}

	size_t MaterialInstanceData::GetPropertyIndexFromResourceIndex(size_t index) const
	{
		return m_pBaseMaterial->GetPropertyIndexFromResourceIndex(index);
	}

	size_t MaterialInstanceData::GetResourcePropertyCount() const
	{
		return m_pBaseMaterial->GetResourcePropertyCount();
	}

	MaterialPropertyInfo* MaterialInstanceData::GetResourcePropertyInfo(size_t index)
	{
		return m_pBaseMaterial->GetResourcePropertyInfo(index);
	}
}
