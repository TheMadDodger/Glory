#pragma once
#include "UUID.h"
#include "AssetManager.h"
#include "ResourceType.h"

namespace Glory
{
	class AssetReferenceBase
	{
	public:
		AssetReferenceBase();
		AssetReferenceBase(UUID uuid);
		virtual ~AssetReferenceBase();

		const UUID AssetUUID() const;
		UUID* AssetUUIDMember();
		void SetUUID(UUID uuid);
		Resource* GetResource();

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

		T* Get()
		{
			return AssetManager::GetOrLoadAsset<T>(m_AssetUUID);
		}

		T* GetImmediate()
		{
			return AssetManager::GetAssetImmediate<T>(m_AssetUUID);
		}
	};
}
