#include "AssetReference.h"
#include "Resources.h"

#include "ResourceReferencing.h"

namespace Glory
{
	ResourceReferenceBase::ResourceReferenceBase(): m_AssetUUID(0)
	{
	}

	ResourceReferenceBase::ResourceReferenceBase(UUID uuid): m_AssetUUID(uuid)
	{
		if (uuid)
			AddResourceReference(uuid);
	}

	ResourceReferenceBase::ResourceReferenceBase(const ResourceReferenceBase& other): m_AssetUUID(other.m_AssetUUID)
	{
		if (m_AssetUUID)
			AddResourceReference(m_AssetUUID);
	}

	ResourceReferenceBase& ResourceReferenceBase::operator=(const ResourceReferenceBase& other)
	{
		m_AssetUUID = other.m_AssetUUID;
		if (m_AssetUUID)
			AddResourceReference(m_AssetUUID);
		return *this;
	}

	ResourceReferenceBase& ResourceReferenceBase::operator=(UUID uuid)
	{
		if (m_AssetUUID)
			RemoveResourceReference(m_AssetUUID);
		m_AssetUUID = uuid;
		if (uuid)
			AddResourceReference(uuid);
		return *this;
	}

	ResourceReferenceBase::ResourceReferenceBase(ResourceReferenceBase&& other) noexcept: m_AssetUUID(other.m_AssetUUID)
	{
		other.m_AssetUUID = 0;
	}

	ResourceReferenceBase& ResourceReferenceBase::operator=(ResourceReferenceBase&& other) noexcept
	{
		m_AssetUUID = other.m_AssetUUID;
		other.m_AssetUUID = 0;
		return *this;
	}

	ResourceReferenceBase::~ResourceReferenceBase()
	{
		if (m_AssetUUID)
			RemoveResourceReference(m_AssetUUID);
		m_AssetUUID = 0;
	}

	const UUID ResourceReferenceBase::GetUUID() const
	{
		return m_AssetUUID;
	}

	void ResourceReferenceBase::SetUUID(UUID uuid)
	{
		if (m_AssetUUID)
			RemoveResourceReference(m_AssetUUID);
		m_AssetUUID = uuid;
		if (uuid)
			AddResourceReference(uuid);
	}

	Resource* ResourceReferenceBase::GetResource(Resources* pResources) const
	{
		return pResources->GetResource(m_AssetUUID);
	}
	
	ResourceReferenceBase::operator bool() const
	{
		return m_AssetUUID;
	}

	void ResourceReferenceBase::ManualRegisterReference() const
	{
		if (m_AssetUUID)
			AddResourceReference(m_AssetUUID);
	}

	WeakResourceReference::WeakResourceReference(): m_AssetUUID(0)
	{
	}

	WeakResourceReference::WeakResourceReference(UUID uuid): m_AssetUUID(uuid)
	{
	}

	WeakResourceReference::WeakResourceReference(const ResourceReferenceBase& other): m_AssetUUID(other.GetUUID())
	{
	}

	WeakResourceReference::~WeakResourceReference()
	{
		m_AssetUUID = 0ull;
	}

	const UUID WeakResourceReference::GetUUID() const
	{
		return m_AssetUUID;
	}

	Resource* WeakResourceReference::GetResource(Resources* pResources) const
	{
		return pResources->GetResource(m_AssetUUID);
	}

	WeakResourceReference::operator bool() const
	{
		return m_AssetUUID;
	}
}
