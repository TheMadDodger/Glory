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
		static Resource* GetAssetImmediate(UUID uuid);

	private:
		static Resource* FindResource(UUID uuid);
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
		static ThreadedUMap<UUID, Resource*> m_pLoadedAssets;
		static ThreadedUMap<std::string, size_t> m_PathToGroupIndex;
		static ThreadedVector<AssetGroup*> m_LoadedAssetGroups;
		static ThreadedQueue<CallbackData> m_ResourceLoadedCallbacks;
		static ThreadedUMap<UUID, std::vector<std::function<void(Resource*)>>> m_AssetLoadedCallbacks;
		static Jobs::JobPool<bool, UUID>* m_pResourceLoadingPool;
	};
}
