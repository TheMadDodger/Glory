#pragma once
#include "UUID.h"
#include "Resource.h"
#include "AssetGroup.h"
#include "ThreadedVar.h"
#include "JobManager.h"

#include <unordered_map>
#include <vector>

namespace Glory
{
	class Engine;

	struct CallbackData
	{
		CallbackData();
		CallbackData(UUID uuid, Resource* pResource);

		UUID m_UUID;
		Resource* m_pResource;
	};

	class AssetManager
	{
	public:
		virtual ~AssetManager();

		void GetAsset(UUID uuid, std::function<void(Resource*)> callback);
		Resource* GetOrLoadAsset(UUID uuid);

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

		Resource* GetAssetImmediate(UUID uuid);

		void ReloadAsset(UUID uuid);
		void UnloadAsset(UUID uuid);
		Resource* FindResource(UUID uuid);
		void AddLoadedResource(Resource* pResource, UUID uuid);
		void AddLoadedResource(Resource* pResource);

		bool IsLoading(UUID uuid);
		void GetAllLoading(std::vector<UUID>& out);

	private:
		bool LoadResourceJob(UUID uuid);
		Resource* LoadAsset(UUID uuid);

	private:
		AssetManager(Engine* pEngine);

		void Initialize();
		void Destroy();
		void RunCallbacks();

	private:
		friend class Engine;
		friend class AssetDatabase;

		struct LoadingLock
		{
			LoadingLock(AssetManager* pManager, UUID uuid);
			~LoadingLock();

			AssetManager* m_pManager;
			bool IsValid;
			UUID m_UUID;
		};

		Engine* m_pEngine;
		ThreadedUMap<UUID, Resource*> m_pLoadedAssets;
		ThreadedVector<UUID> m_pLoadingAssets;
		ThreadedUMap<std::string, size_t> m_PathToGroupIndex;
		ThreadedVector<AssetGroup*> m_LoadedAssetGroups;
		ThreadedQueue<CallbackData> m_ResourceLoadedCallbacks;
		ThreadedUMap<UUID, std::vector<std::function<void(Resource*)>>> m_AssetLoadedCallbacks;
		Jobs::JobPool<bool, UUID>* m_pResourceLoadingPool;
	};
}
