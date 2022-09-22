#include "AssetManager.h"
#include "AssetDatabase.h"
#include "GloryContext.h"
#include "Engine.h"

namespace Glory
{
	void AssetManager::GetAsset(UUID uuid, std::function<void(Resource*)> callback)
	{
		Resource* pResource = FindResource(uuid);
		if (pResource)
		{
			callback(pResource);
			return;
		}

		if (ASSET_MANAGER->m_AssetLoadedCallbacks.Contains(uuid))
		{
			if (callback != NULL)
			{
				ASSET_MANAGER->m_AssetLoadedCallbacks.Do(uuid, [&](std::vector<std::function<void(Resource*)>>* callbacks)
				{
					callbacks->push_back(callback);
				});
			}
			return;
		}
		else ASSET_MANAGER->m_AssetLoadedCallbacks.Set(uuid, { callback });

		ASSET_MANAGER->m_pResourceLoadingPool->QueueSingleJob(AssetManager::LoadResourceJob, uuid);
	}

	Resource* AssetManager::GetOrLoadAsset(UUID uuid)
	{
		Resource* pResource = FindResource(uuid);
		if (pResource) return pResource;
		if (ASSET_MANAGER->m_AssetLoadedCallbacks.Contains(uuid)) return nullptr;
		ASSET_MANAGER->m_AssetLoadedCallbacks.Set(uuid, { [](Resource*) {} });
		ASSET_MANAGER->m_pResourceLoadingPool->QueueSingleJob(AssetManager::LoadResourceJob, uuid);
		return nullptr;
	}

	Resource* AssetManager::GetAssetImmediate(UUID uuid)
	{
		Resource* pResource = FindResource(uuid);
		if (pResource) return pResource;
		return LoadAsset(uuid);
	}

	Resource* AssetManager::FindResource(UUID uuid)
	{
		if (!ASSET_MANAGER->m_pLoadedAssets.Contains(uuid)) return nullptr;
		return ASSET_MANAGER->m_pLoadedAssets[uuid];
	}

	bool AssetManager::LoadResourceJob(UUID uuid)
	{
		Resource* pResource = LoadAsset(uuid);
		ASSET_MANAGER->m_ResourceLoadedCallbacks.push(CallbackData(uuid, pResource));
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

		if (!std::filesystem::exists(path))
			path = assetLocation.m_Path;

		Resource* pResource = pModule->LoadUsingAny(path.string(), meta.ImportSettings());
		if (pResource == nullptr)
		{
			Debug::LogError("Failed to load asset: " + std::to_string(uuid) + " at path: " + path.string());
			return nullptr;
		}

		pResource->m_ID = uuid;
		ASSET_MANAGER->m_pLoadedAssets.Set(uuid, pResource);
		ASSET_DATABASE->m_Callbacks.EnqueueCallback(CallbackType::CT_AssetLoaded, uuid, pResource);
		return pResource;
	}

	AssetManager::AssetManager() {}

	AssetManager::~AssetManager() {}

	void AssetManager::Initialize()
	{
		AssetDatabase::Initialize();

		ASSET_MANAGER->m_pResourceLoadingPool = Jobs::JobManager::Run<bool, UUID>();
	}

	void AssetManager::Destroy()
	{
		ASSET_MANAGER->m_pLoadedAssets.ForEach([](const UUID& key, Resource* value)
		{
			delete value;
		});

		ASSET_MANAGER->m_LoadedAssetGroups.ForEach([](AssetGroup* value)
		{
			delete value;
		});

		ASSET_MANAGER->m_pLoadedAssets.Clear();
		ASSET_MANAGER->m_LoadedAssetGroups.Clear();
		ASSET_MANAGER->m_PathToGroupIndex.Clear();
		AssetDatabase::Destroy();

		ASSET_MANAGER->m_pResourceLoadingPool = nullptr;
	}

	void AssetManager::RunCallbacks()
	{
		ASSET_DATABASE->m_Callbacks.RunCallbacks();

		CallbackData callbackData;
		while (ASSET_MANAGER->m_ResourceLoadedCallbacks.Pop(callbackData))
		{
			ASSET_MANAGER->m_AssetLoadedCallbacks.Do(callbackData.m_UUID, [&](const std::vector<std::function<void(Resource*)>>& callbacks)
			{
				for (size_t i = 0; i < callbacks.size(); i++)
				{
					callbacks[i](callbackData.m_pResource);
				}
			});
			ASSET_MANAGER->m_AssetLoadedCallbacks.Erase(callbackData.m_UUID);
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
