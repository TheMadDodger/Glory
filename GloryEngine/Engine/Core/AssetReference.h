#pragma once
#include "UUID.h"
#include "ResourceType.h"
#include "Engine.h"
#include "Resources.h"

#include <Reflection.h>

namespace Glory
{
	class Resource;
	class Resources;

	class AssetReferenceBase
	{
	public:
		AssetReferenceBase();
		AssetReferenceBase(UUID uuid);
		virtual ~AssetReferenceBase();

		const UUID AssetUUID() const;
		UUID* AssetUUIDMember();
		void SetUUID(UUID uuid);

		virtual const uint32_t TypeHash() { return 0; }
		virtual Resource* GetBase(Resources*) const { return nullptr; }

		virtual AssetReferenceBase* CreateCopy() { return new AssetReferenceBase(m_AssetUUID); };

	protected:
		REFLECTABLE(AssetReferenceBase, (UUID) (m_AssetUUID))
	};

	template<class T>
	class AssetReference : public AssetReferenceBase
	{
	public:
		AssetReference() : AssetReferenceBase() {}
		AssetReference(UUID uuid) : AssetReferenceBase(uuid) {}
		AssetReference(T* pAsset) : AssetReferenceBase(pAsset ? pAsset->GetUUID() : 0) {}
		virtual ~AssetReference() {}

		virtual const uint32_t TypeHash() override
		{
			static const uint32_t typeHash = ResourceTypes::GetHash<T>();
			return typeHash;
		}

		Resource* GetBase(Resources* pResources) const override
		{
			return Get(pResources);
		}

		AssetReferenceBase* CreateCopy() override
		{
			return new AssetReference<T>(m_AssetUUID);
		}

		T* Get(Resources* pResources) const
		{
			return pResources->Manager<T>()->Get(m_AssetUUID);
		}
	};
}
