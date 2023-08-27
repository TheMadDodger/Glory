#pragma once
#include "UUID.h"
#include "Resource.h"
#include "AssetGroup.h"
#include "Game.h"
#include "AssetDatabase.h"
#include "ThreadedVar.h"
#include "JobManager.h"

#include <unordered_map>
#include <vector>

namespace Glory
{
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
		static void GetAsset(UUID uuid, std::function<void(Resource*)> callback);
		static Resource* GetOrLoadAsset(UUID uuid);

		template<class T>
		static T* GetOrLoadAsset(UUID uuid)
		{
			Resource* pResource = GetOrLoadAsset(uuid);
			if (!pResource) return nullptr;
			return (T*)pResource;
		}

		template<class T>
		static T* GetAssetImmediate(UUID uuid)
		{
			Resource* pResource = GetAssetImmediate(uuid);
			if (!pResource) return nullptr;
			return (T*)pResource;
		}

		static Resource* GetAssetImmediate(UUID uuid);

		static void ReloadAsset(UUID uuid);
		static void UnloadAsset(UUID uuid);
		static Resource* FindResource(UUID uuid);
		static void AddLoadedResource(Resource* pResource);

		static bool IsLoading(UUID uuid);
		static void GetAllLoading(std::vector<UUID>& out);

	private:
		static bool LoadResourceJob(UUID uuid);
		static Resource* LoadAsset(UUID uuid);

	private:
		AssetManager();
		virtual ~AssetManager();

		static void Initialize();
		static void Destroy();
		static void RunCallbacks();

	private:
		friend class Engine;
		friend class AssetDatabase;
		friend class GloryContext;

		struct LoadingLock
		{
			LoadingLock(UUID uuid);
			~LoadingLock();

			bool IsValid;
			UUID m_UUID;
		};

		ThreadedUMap<UUID, Resource*> m_pLoadedAssets;
		ThreadedVector<UUID> m_pLoadingAssets;
		ThreadedUMap<std::string, size_t> m_PathToGroupIndex;
		ThreadedVector<AssetGroup*> m_LoadedAssetGroups;
		ThreadedQueue<CallbackData> m_ResourceLoadedCallbacks;
		ThreadedUMap<UUID, std::vector<std::function<void(Resource*)>>> m_AssetLoadedCallbacks;
		Jobs::JobPool<bool, UUID>* m_pResourceLoadingPool;
	};
}
