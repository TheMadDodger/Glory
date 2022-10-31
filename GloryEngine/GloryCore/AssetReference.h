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

		const UUID AssetUUID();
		UUID* AssetUUIDMember();
		void SetUUID(UUID uuid);
		Resource* GetResource();

		virtual const size_t TypeHash() = 0;

	protected:
		UUID m_AssetUUID;
	};

	template<class T>
	class AssetReference : public AssetReferenceBase
	{
	public:
		AssetReference() : AssetReferenceBase() {}
		AssetReference(UUID uuid) : AssetReferenceBase(uuid) {}
		virtual ~AssetReference() {}

		virtual const size_t TypeHash() override
		{
			static const size_t TYPE_HASH = ResourceType::GetHash<T>();
			return TYPE_HASH;
		}

		T* Get()
		{
			return AssetManager::GetOrLoadAsset<T>(m_AssetUUID);
		}
	};
}
