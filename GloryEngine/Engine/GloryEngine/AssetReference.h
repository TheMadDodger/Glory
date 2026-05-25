#pragma once
#include "ResourceType.h"

#include <engine_visibility.h>

#include <UUID.h>

#include <Reflection.h>

namespace Glory
{
	class Resource;
	class Resources;

	class ResourceReferenceBase
	{
	public:
		GLORY_ENGINE_API ResourceReferenceBase();
		GLORY_ENGINE_API ResourceReferenceBase(UUID uuid);

		GLORY_ENGINE_API ResourceReferenceBase(const ResourceReferenceBase& other);
		GLORY_ENGINE_API ResourceReferenceBase& operator=(const ResourceReferenceBase& other);
		GLORY_ENGINE_API ResourceReferenceBase& operator=(UUID uuid);

		GLORY_ENGINE_API ResourceReferenceBase(ResourceReferenceBase&& other) noexcept;
		GLORY_ENGINE_API ResourceReferenceBase& operator=(ResourceReferenceBase&& other) noexcept;

		GLORY_ENGINE_API virtual ~ResourceReferenceBase();

		GLORY_ENGINE_API const UUID GetUUID() const;
		GLORY_ENGINE_API void SetUUID(UUID uuid);
		GLORY_ENGINE_API Resource* GetResource(Resources* pResources) const;

		virtual const uint32_t TypeHash() { return 0; };

		virtual ResourceReferenceBase* CreateCopy() { return new ResourceReferenceBase(m_AssetUUID); };

		GLORY_ENGINE_API operator bool() const;
		GLORY_ENGINE_API void ManualRegisterReference() const;

	protected:
		REFLECTABLE(ResourceReferenceBase, (UUID) (m_AssetUUID));
	};

	class WeakResourceReference
	{
	public:
		GLORY_ENGINE_API WeakResourceReference();
		GLORY_ENGINE_API WeakResourceReference(UUID uuid);
		GLORY_ENGINE_API WeakResourceReference(const ResourceReferenceBase& other);

		GLORY_ENGINE_API WeakResourceReference(const WeakResourceReference&) noexcept = default;
		GLORY_ENGINE_API WeakResourceReference& operator=(const WeakResourceReference&) noexcept = default;

		GLORY_ENGINE_API virtual ~WeakResourceReference();

		GLORY_ENGINE_API const UUID GetUUID() const;
		GLORY_ENGINE_API Resource* GetResource(Resources* pResources) const;

		GLORY_ENGINE_API operator bool() const;

	protected:
		REFLECTABLE(WeakResourceReference, (UUID)(m_AssetUUID));
	};

	template<class T>
	class ResourceReference : public ResourceReferenceBase
	{
	public:
		ResourceReference() : ResourceReferenceBase() {}
		ResourceReference(UUID uuid) : ResourceReferenceBase(uuid) {}
		ResourceReference(uint64_t id) : ResourceReferenceBase(id) {}
		ResourceReference(T* pAsset) : ResourceReferenceBase(pAsset ? pAsset->GetUUID() : UUID(0ull)) {}

		ResourceReference(const ResourceReference&) = default;
		ResourceReference& operator=(const ResourceReference&) = default;
		ResourceReference& operator=(UUID uuid)
		{
			ResourceReferenceBase& base = static_cast<ResourceReferenceBase&>(*this);
			base = uuid;
			return *this;
		}

		ResourceReference(ResourceReference&&) = default;
		ResourceReference& operator=(ResourceReference&&) = default;

		virtual ~ResourceReference() {}

		virtual const uint32_t TypeHash() override
		{
			static const uint32_t typeHash = ResourceTypes::GetHash<T>();
			return typeHash;
		}

		ResourceReferenceBase* CreateCopy() override
		{
			return new ResourceReference<T>(m_AssetUUID);
		}

		T* Get(Resources* pResources) const
		{
			Resource* pResource = GetResource(pResources);
			return pResource ? dynamic_cast<T*>(pResource) : nullptr;
		}
	};
}
