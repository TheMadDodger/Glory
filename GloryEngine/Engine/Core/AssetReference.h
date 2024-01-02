#pragma once
#include "UUID.h"
#include "ResourceType.h"

#include <Reflection.h>

namespace Glory
{
	class Resource;
	class AssetManager;

	class AssetReferenceBase
	{
	public:
		AssetReferenceBase();
		AssetReferenceBase(UUID uuid);
		virtual ~AssetReferenceBase();

		const UUID AssetUUID() const;
		UUID* AssetUUIDMember();
		void SetUUID(UUID uuid);
		Resource* GetResource(AssetManager* pAssets) const;
		Resource* GetResourceImmediate(AssetManager* pAssets) const;

		virtual const uint32_t TypeHash() { return 0; };

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
			static const uint32_t typeHash = ResourceType::GetHash<T>();
			return typeHash;
		}

		AssetReferenceBase* CreateCopy() override
		{
			return new AssetReference<T>(m_AssetUUID);
		}

		T* Get(AssetManager* pManager) const
		{
			Resource* pResource = GetResource(pManager);
			return pResource ? dynamic_cast<T*>(pResource) : nullptr;
		}

		T* GetImmediate(AssetManager* pManager) const
		{
			Resource* pResource = GetResourceImmediate(pManager);
			return pResource ? dynamic_cast<T*>(pResource) : nullptr;
		}
	};
}
