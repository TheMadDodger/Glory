#include "AssetManager.h"
#include "AssetDatabase.h"

namespace Glory
{
	ThreadedUMap<UUID, Resource*> AssetManager::m_pLoadedAssets;
	ThreadedUMap<std::string, size_t> AssetManager::m_PathToGroupIndex;
	ThreadedVector<AssetGroup*> AssetManager::m_LoadedAssetGroups;
	Jobs::JobPool<bool, UUID>* AssetManager::m_pResourceLoadingPool = nullptr;
	ThreadedQueue<CallbackData> AssetManager::m_ResourceLoadedCallbacks;
	ThreadedUMap<UUID, std::vector<std::function<void(Resource*)>>> AssetManager::m_AssetLoadedCallbacks;

	void AssetManager::GetAsset(UUID uuid, std::function<void(Resource*)> callback)
	{
		Resource* pResource = FindResource(uuid);
		if (pResource)
		{
			callback(pResource);
			return;
		}

		if (m_AssetLoadedCallbacks.Contains(uuid))
		{
			if (callback != NULL)
			{
				m_AssetLoadedCallbacks.Do(uuid, [&](std::vector<std::function<void(Resource*)>>* callbacks)
				{
					callbacks->push_back(callback);
				});
			}
			return;
		}
		else m_AssetLoadedCallbacks.Set(uuid, { callback });

		m_pResourceLoadingPool->QueueSingleJob(AssetManager::LoadResourceJob, uuid);
	}

	Resource* AssetManager::GetAssetImmediate(UUID uuid)
	{
		Resource* pResource = FindResource(uuid);
		if (pResource) return pResource;
		return LoadAsset(uuid);
	}

	Resource* AssetManager::FindResource(UUID uuid)
	{
		if (!m_pLoadedAssets.Contains(uuid)) return nullptr;
		return m_pLoadedAssets[uuid];
	}

	bool AssetManager::LoadResourceJob(UUID uuid)
	{
		Resource* pResource = LoadAsset(uuid);
		m_ResourceLoadedCallbacks.push(CallbackData(uuid, pResource));
		return pResource;
	}

	Resource* AssetManager::LoadAsset(UUID uuid)
	{
		ResourceMeta meta;
		AssetDatabase::GetResourceMeta(uuid, meta);
		AssetLocation assetLocation;
		if (!AssetDatabase::GetAssetLocation(uuid, assetLocation)) return nullptr;

		LoaderModule* pModule = Game::GetGame().GetEngine()->GetLoaderModule(meta.Hash());

		if (pModule == nullptr) return nullptr;

		if (assetLocation.m_IsSubAsset)
		{
			throw new std::exception("Not implemented yet");
		}

		std::filesystem::path path = Game::GetAssetPath();
		path.append(assetLocation.m_Path);

		Resource* pResource = pModule->LoadUsingAny(path.string(), meta.ImportSettings());
		pResource->m_ID = uuid;
		m_pLoadedAssets.Set(uuid, pResource);
		AssetDatabase::m_Callbacks.EnqueueCallback(CallbackType::CT_AssetLoaded, uuid, pResource);
		return pResource;
	}

	AssetManager::AssetManager() {}

	AssetManager::~AssetManager() {}

	void AssetManager::Initialize()
	{
		AssetDatabase::Initialize();

		m_pResourceLoadingPool = Jobs::JobManager::Run<bool, UUID>();
	}

	void AssetManager::Destroy()
	{
		m_pLoadedAssets.ForEach([](const UUID& key, Resource* value)
		{
			delete value;
		});

		m_LoadedAssetGroups.ForEach([](AssetGroup* value)
		{
			delete value;
		});

		m_pLoadedAssets.Clear();
		m_LoadedAssetGroups.Clear();
		m_PathToGroupIndex.Clear();
		AssetDatabase::Destroy();

		m_pResourceLoadingPool = nullptr;
	}

	void AssetManager::RunCallbacks()
	{
		AssetDatabase::m_Callbacks.RunCallbacks();

		CallbackData callbackData;
		while (m_ResourceLoadedCallbacks.Pop(callbackData))
		{
			m_AssetLoadedCallbacks.Do(callbackData.m_UUID, [&](const std::vector<std::function<void(Resource*)>>& callbacks)
			{
				for (size_t i = 0; i < callbacks.size(); i++)
				{
					callbacks[i](callbackData.m_pResource);
				}
			});
			m_AssetLoadedCallbacks.Erase(callbackData.m_UUID);
		}
	}

	CallbackData::CallbackData()
		: m_UUID(0), m_pResource(nullptr)
	{
	}

	CallbackData::CallbackData(UUID uuid, Resource* pResource)
		: m_UUID(uuid), m_pResource(pResource)
	{
	}
}
