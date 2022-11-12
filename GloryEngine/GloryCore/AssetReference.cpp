#include "AssetReference.h"

namespace Glory
{
	AssetReferenceBase::AssetReferenceBase() : m_AssetUUID(0)
	{
	}

	AssetReferenceBase::AssetReferenceBase(UUID uuid) : m_AssetUUID(uuid)
	{
	}

	AssetReferenceBase::~AssetReferenceBase()
	{
		m_AssetUUID = 0;
	}

	const UUID AssetReferenceBase::AssetUUID()
	{
		return m_AssetUUID;
	}

	UUID* AssetReferenceBase::AssetUUIDMember()
	{
		return &m_AssetUUID;
	}

	void AssetReferenceBase::SetUUID(UUID uuid)
	{
		m_AssetUUID = uuid;
	}

	Resource* AssetReferenceBase::GetResource()
	{
		return AssetManager::GetOrLoadAsset(m_AssetUUID);
	}

	bool* AssetReferenceBase::IsDirty()
	{
		return &m_IsDirty;
	}
}
