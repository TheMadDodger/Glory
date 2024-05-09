#pragma once
#include "UUID.h"
#include "Resource.h"
#include "ThreadedVar.h"
#include "JobManager.h"

#include <unordered_map>
#include <vector>

namespace Glory
{
	class Engine;

	class AssetManager
	{
	public:
		AssetManager(Engine* pEngine);
		virtual ~AssetManager();

		template<class T>
		T* GetOrLoadAsset(UUID uuid)
		{
			Resource* pResource = GetOrLoadAsset(uuid);
			if (!pResource) return nullptr;
			return (T*)pResource;
		}

		template<class T>
		T* GetAssetImmediate(UUID uuid)
		{
			Resource* pResource = GetAssetImmediate(uuid);
			if (!pResource) return nullptr;
			return (T*)pResource;
		}

		virtual Resource* GetAssetImmediate(UUID uuid) = 0;
		virtual void GetAsset(UUID uuid, std::function<void(Resource*)> callback) = 0;
		virtual Resource* GetOrLoadAsset(UUID uuid) = 0;
		virtual void UnloadAsset(UUID uuid) = 0;
		virtual Resource* FindResource(UUID uuid) = 0;
		virtual void AddLoadedResource(Resource* pResource, UUID uuid) = 0;
		virtual void AddLoadedResource(Resource* pResource) = 0;

		virtual void Initialize() = 0;
		virtual void Destroy() = 0;

	protected:
		Engine* m_pEngine;

	private:
		friend class Engine;
		friend class AssetDatabase;

	};
}
