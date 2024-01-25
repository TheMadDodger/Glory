#include "MaterialInstanceData.h"
#include "ResourceType.h"
#include "BinaryStream.h"
#include "MaterialManager.h"

#include <algorithm>

namespace Glory
{
	MaterialInstanceData::MaterialInstanceData() : m_BaseMaterial(0)
	{
		APPEND_TYPE(MaterialInstanceData);
	}

	MaterialInstanceData::MaterialInstanceData(UUID uuid, const std::string& name):
		MaterialData(uuid, name)
	{
		APPEND_TYPE(MaterialInstanceData);
	}

	MaterialInstanceData::MaterialInstanceData(UUID baseMaterial) : m_BaseMaterial(baseMaterial)
	{
		APPEND_TYPE(MaterialInstanceData);
	}

	MaterialInstanceData::~MaterialInstanceData()
	{
	}

	size_t MaterialInstanceData::ShaderCount(const MaterialManager& manager) const
	{
		MaterialData* pBaseMaterial = GetBaseMaterial(manager);
		if (!pBaseMaterial) return 0;
		return pBaseMaterial->ShaderCount(manager);
	}

	UUID MaterialInstanceData::GetShaderIDAt(const MaterialManager& manager, size_t index) const
	{
		MaterialData* pBaseMaterial = GetBaseMaterial(manager);
		if (!pBaseMaterial) return 0;
		return pBaseMaterial->GetShaderIDAt(manager, index);
	}

	UUID MaterialInstanceData::BaseMaterialID() const
	{
		return m_BaseMaterial;
	}

	MaterialData* MaterialInstanceData::GetBaseMaterial(const MaterialManager& manager) const
	{
		return m_BaseMaterial ? manager.GetMaterial(m_BaseMaterial) : nullptr;
	}

	UUID MaterialInstanceData::GetGPUUUID() const
	{
		return m_BaseMaterial;
	}

	void MaterialInstanceData::CopyOverrideStates(std::vector<bool>& destination)
	{
		if (m_PropertyOverridesEnable.size() != destination.size()) destination.resize(m_PropertyOverridesEnable.size());
		destination.assign(m_PropertyOverridesEnable.begin(), m_PropertyOverridesEnable.end());
	}

	void MaterialInstanceData::PasteOverrideStates(std::vector<bool>& destination)
	{
		if (m_PropertyOverridesEnable.size() != destination.size()) m_PropertyOverridesEnable.resize(destination.size());
		m_PropertyOverridesEnable.assign(destination.begin(), destination.end());
	}

	size_t MaterialInstanceData::PropertyInfoCount(const MaterialManager& manager) const
	{
		MaterialData* pBaseMaterial = GetBaseMaterial(manager);
		if (!pBaseMaterial) return 0;
		return pBaseMaterial->PropertyInfoCount(manager);
	}

	MaterialPropertyInfo* MaterialInstanceData::GetPropertyInfoAt(const MaterialManager& manager, size_t index)
	{
		MaterialData* pBaseMaterial = GetBaseMaterial(manager);
		if (!pBaseMaterial) return nullptr;
		return pBaseMaterial->GetPropertyInfoAt(manager, index);
	}

	size_t MaterialInstanceData::GetCurrentBufferOffset(const MaterialManager& manager) const
	{
		MaterialData* pBaseMaterial = GetBaseMaterial(manager);
		if (!pBaseMaterial) return 0;
		return pBaseMaterial->GetCurrentBufferOffset(manager);
	}

	std::vector<char>& MaterialInstanceData::GetBufferReference(const MaterialManager& manager)
	{
		MaterialData* pBaseMaterial = GetBaseMaterial(manager);
		if (!pBaseMaterial) return m_PropertyBuffer;
		std::vector<char>& baseBuffer = pBaseMaterial->GetBufferReference(manager);
		m_PropertyBuffer.resize(baseBuffer.size());
		for (size_t i = 0; i < pBaseMaterial->PropertyInfoCount(manager); i++)
		{
			MaterialPropertyInfo* propertyInfo = pBaseMaterial->GetPropertyInfoAt(manager, i);
			if (propertyInfo->IsResource()) continue;
			if (m_PropertyOverridesEnable.size() <= i || m_PropertyOverridesEnable[i]) continue;
			memcpy(&m_PropertyBuffer[propertyInfo->Offset()], &baseBuffer[propertyInfo->Offset()], propertyInfo->Size());
		}
		return m_PropertyBuffer;
	}

	bool MaterialInstanceData::GetPropertyInfoIndex(const MaterialManager& manager, const std::string& name, size_t& index) const
	{
		MaterialData* pBaseMaterial = GetBaseMaterial(manager);
		if (!pBaseMaterial) return false;
		return pBaseMaterial->GetPropertyInfoIndex(manager, name, index);
	}

	AssetReference<TextureData>* MaterialInstanceData::GetResourceUUIDPointer(MaterialManager& manager, size_t index)
	{
		MaterialData* pBaseMaterial = GetBaseMaterial(manager);
		if (!pBaseMaterial) return &m_Resources[index];
		const size_t propertyIndex = GetPropertyIndexFromResourceIndex(manager, index);
		return m_PropertyOverridesEnable[propertyIndex] ? &m_Resources[index] : pBaseMaterial->GetResourceUUIDPointer(manager, index);
	}

	size_t MaterialInstanceData::GetPropertyIndexFromResourceIndex(MaterialManager& manager, size_t index) const
	{
		MaterialData* pBaseMaterial = GetBaseMaterial(manager);
		if (!pBaseMaterial) return 0;
		return pBaseMaterial->GetPropertyIndexFromResourceIndex(manager, index);
	}

	size_t MaterialInstanceData::GetResourcePropertyCount(MaterialManager& manager) const
	{
		MaterialData* pBaseMaterial = GetBaseMaterial(manager);
		if (!pBaseMaterial) return 0;
		return pBaseMaterial->GetResourcePropertyCount(manager);
	}

	MaterialPropertyInfo* MaterialInstanceData::GetResourcePropertyInfo(MaterialManager& manager, size_t index)
	{
		MaterialData* pBaseMaterial = GetBaseMaterial(manager);
		if (!pBaseMaterial) return nullptr;
		return pBaseMaterial->GetResourcePropertyInfo(manager, index);
	}

	void MaterialInstanceData::Resize(const MaterialManager& manager, MaterialData* pBase)
	{
		if (!pBase) return;
		if (!pBase || m_PropertyBuffer.size() == pBase->GetBufferReference(manager).size()) return;
		m_PropertyOverridesEnable.resize(pBase->PropertyInfoCount(manager), false);
		m_PropertyBuffer.resize(pBase->GetBufferReference(manager).size());
		m_Resources.resize(pBase->ResourceCount());
	}

	bool MaterialInstanceData::IsPropertyOverriden(size_t index) const
	{
		return m_PropertyOverridesEnable[index];
	}

	void MaterialInstanceData::EnableProperty(size_t index)
	{
		m_PropertyOverridesEnable[index] = true;
	}

	void MaterialInstanceData::DisableProperty(size_t index)
	{
		m_PropertyOverridesEnable[index] = false;
	}

	void MaterialInstanceData::Serialize(BinaryStream& container) const
	{
		/* Write base material */
		container.Write(m_BaseMaterial);

		/* Write overrides */
		container.Write(m_PropertyOverridesEnable.size());
		for (size_t i = 0; i < m_PropertyOverridesEnable.size(); ++i)
		{
			container.Write(m_PropertyOverridesEnable[i]);
		}

		/* Write property buffer */
		container.Write(m_PropertyBuffer.size()).
			Write(m_PropertyBuffer.data(), m_PropertyBuffer.size());

		/* Write resources */
		container.Write(m_Resources.size());
		for (size_t i = 0; i < m_Resources.size(); ++i)
		{
			container.Write(m_Resources[i].AssetUUID());
		}
	}

	void MaterialInstanceData::Deserialize(BinaryStream& container)
	{
		/* Read base material */
		container.Read(m_BaseMaterial);

		/* Write overrides */
		size_t numPropertyOverrides;
		container.Read(numPropertyOverrides);
		m_PropertyOverridesEnable.resize(numPropertyOverrides);
		for (size_t i = 0; i < m_PropertyOverridesEnable.size(); ++i)
		{
			container.Read(m_PropertyOverridesEnable[i]);
		}

		/* Read property buffer */
		size_t propertyBufferSize;
		container.Read(propertyBufferSize);
		m_PropertyBuffer.resize(propertyBufferSize);
		container.Read(m_PropertyBuffer.data(), propertyBufferSize);

		/* Read resources */
		size_t numResources;
		container.Read(numResources);
		m_Resources.resize(numResources);
		for (size_t i = 0; i < m_Resources.size(); ++i)
		{
			container.Read(*m_Resources[i].AssetUUIDMember());
		}
	}

	std::vector<char>& MaterialInstanceData::GetPropertyBuffer(MaterialManager& manager, size_t index)
	{
		MaterialData* pBaseMaterial = GetBaseMaterial(manager);
		if (!pBaseMaterial) return m_PropertyBuffer;
		if (!m_PropertyOverridesEnable[index]) return pBaseMaterial->m_PropertyBuffer;
		return m_PropertyBuffer;
	}
}
