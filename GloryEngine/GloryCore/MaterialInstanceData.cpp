#include "MaterialInstanceData.h"
#include "ResourceType.h"
#include <algorithm>

namespace Glory
{
	MaterialInstanceData::MaterialInstanceData() : m_pBaseMaterial(nullptr)
	{
	}

	MaterialInstanceData::MaterialInstanceData(MaterialData* pBaseMaterial) : m_pBaseMaterial(pBaseMaterial)
	{
		APPEND_TYPE(MaterialInstanceData);

		if (!pBaseMaterial) return;
		m_PropertyOverridesEnable.resize(pBaseMaterial->PropertyInfoCount(), false);
		m_PropertyBuffer.resize(pBaseMaterial->GetBufferReference().size());
		m_Resources.resize(pBaseMaterial->ResourceCount());
	}

	MaterialInstanceData::~MaterialInstanceData()
	{
		m_pBaseMaterial = nullptr;
	}

	size_t MaterialInstanceData::ShaderCount() const
	{
		if (!m_pBaseMaterial) return 0;
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

	void MaterialInstanceData::SetBaseMaterial(MaterialData* pMaterial)
	{
		m_pBaseMaterial = pMaterial;

		if (m_pBaseMaterial == nullptr)
		{
			m_PropertyOverridesEnable.clear();
			m_PropertyBuffer.clear();
			m_Resources.clear();
			return;
		}

		ReloadProperties();
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
		if (!m_pBaseMaterial) return 0;
		return m_pBaseMaterial->PropertyInfoCount();
	}

	MaterialPropertyInfo* MaterialInstanceData::GetPropertyInfoAt(size_t index)
	{
		return m_pBaseMaterial->GetPropertyInfoAt(index);
	}

	size_t MaterialInstanceData::GetCurrentBufferOffset() const
	{
		if (!m_pBaseMaterial) return 0;
		return m_pBaseMaterial->GetCurrentBufferOffset();
	}

	std::vector<char>& MaterialInstanceData::GetBufferReference()
	{
		if (!m_pBaseMaterial) return m_PropertyBuffer;
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
		return m_pBaseMaterial ? m_pBaseMaterial->GetPropertyInfoIndex(name, index) : false;
	}

	UUID* MaterialInstanceData::GetResourceUUIDPointer(size_t index)
	{
		size_t propertyIndex = GetPropertyIndexFromResourceIndex(index);
		return m_PropertyOverridesEnable[propertyIndex] ? &m_Resources[index] : m_pBaseMaterial->GetResourceUUIDPointer(index);
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

	void MaterialInstanceData::ReloadProperties()
	{
		if (!m_pBaseMaterial || m_PropertyBuffer.size() == m_pBaseMaterial->GetBufferReference().size()) return;
		m_PropertyOverridesEnable.resize(m_pBaseMaterial->PropertyInfoCount(), false);
		m_PropertyBuffer.resize(m_pBaseMaterial->GetBufferReference().size());
		m_Resources.resize(m_pBaseMaterial->ResourceCount());
	}

	void MaterialInstanceData::EnableProperty(size_t index)
	{
		m_PropertyOverridesEnable[index] = true;
	}

	std::vector<char>& MaterialInstanceData::GetPropertyBuffer(size_t index)
	{
		if (!m_PropertyOverridesEnable[index]) return m_pBaseMaterial->m_PropertyBuffer;
		return m_PropertyBuffer;
	}
}
