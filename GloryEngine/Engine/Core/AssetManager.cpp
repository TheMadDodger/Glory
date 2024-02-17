#include "AssetManager.h"
#include "AssetDatabase.h"
#include "Engine.h"
#include "Debug.h"

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
		else if (callback != NULL) m_AssetLoadedCallbacks.Set(uuid, { callback });

		m_pResourceLoadingPool->QueueSingleJob([this](UUID uuid) { return LoadResourceJob(uuid); }, uuid);
	}

	Resource* AssetManager::GetOrLoadAsset(UUID uuid)
	{
		if (m_pLoadingAssets.Contains(uuid)) return nullptr;

		Resource* pResource = FindResource(uuid);
		if (pResource) return pResource;
		if (m_AssetLoadedCallbacks.Contains(uuid)) return nullptr;
		m_AssetLoadedCallbacks.Set(uuid, { [](Resource*) {} });
		m_pResourceLoadingPool->QueueSingleJob([this](UUID uuid) { return LoadResourceJob(uuid); }, uuid);
		return nullptr;
	}

	Resource* AssetManager::GetAssetImmediate(UUID uuid)
	{
		while (true)
		{
			LoadingLock lock{ this, uuid };
			if (lock.IsValid) break;
		}

		Resource* pResource = FindResource(uuid);
		if (pResource) return pResource;
		return LoadAsset(uuid);
	}

	void AssetManager::ReloadAsset(UUID uuid)
	{
		UnloadAsset(uuid);
		GetAsset(uuid, [&](Resource* pResource)
		{
			m_pEngine->GetAssetDatabase().m_Callbacks.EnqueueCallback(CallbackType::CT_AssetReloaded, uuid, pResource);
		});
	}

	void AssetManager::UnloadAsset(UUID uuid)
	{
		if (!m_pLoadedAssets.Contains(uuid)) return;
		m_pLoadedAssets.DoErase(uuid, [](Resource** pResource) { if (*pResource) delete *pResource; });
	}

	Resource* AssetManager::FindResource(UUID uuid)
	{
		if (!m_pLoadedAssets.Contains(uuid)) return nullptr;
		return m_pLoadedAssets[uuid];
	}

	void AssetManager::AddLoadedResource(Resource* pResource, UUID uuid)
	{
		pResource->m_ID = uuid;
		AddLoadedResource(pResource);
	}

	void AssetManager::AddLoadedResource(Resource* pResource)
	{
		UnloadAsset(pResource->GetUUID());
		m_pLoadedAssets.Set(pResource->GetUUID(), pResource);
	}

	bool AssetManager::IsLoading(UUID uuid)
	{
		return m_pLoadingAssets.Contains(uuid);
	}

	void AssetManager::GetAllLoading(std::vector<UUID>& out)
	{
		m_pLoadingAssets.ForEach([&out](const UUID& uuid) {
			out.push_back(uuid);
		});
	}

	bool AssetManager::LoadResourceJob(UUID uuid)
	{
		Resource* pResource = LoadAsset(uuid);
		m_ResourceLoadedCallbacks.push(CallbackData(uuid, pResource));
		return pResource;
	}

	Resource* AssetManager::LoadAsset(UUID uuid)
	{
		LoadingLock lock{ this, uuid };
		if (!lock.IsValid) return nullptr;

		ResourceMeta meta;
		if (!m_pEngine->GetAssetDatabase().GetResourceMeta(uuid, meta)) return nullptr;
		AssetLocation assetLocation;
		if (!m_pEngine->GetAssetDatabase().GetAssetLocation(uuid, assetLocation)) return nullptr;

		//if (!assetLocation.SubresourcePath.empty())
		//{
		//	/* Load root resource */
		//	UUID rootUUID = m_pEngine->GetAssetDatabase().GetAssetUUID(assetLocation.Path);
		//	Resource* pRootResource = FindResource(rootUUID);
		//	if (!pRootResource)
		//		pRootResource = LoadAsset(rootUUID);

		//	if (!pRootResource) return nullptr;
		//	Resource* pSubResource = pRootResource->SubresourceFromPath(assetLocation.SubresourcePath);
		//	if (!pSubResource) return nullptr;

		//	std::filesystem::path namePath = assetLocation.Path;
		//	if (!assetLocation.SubresourcePath.empty()) namePath.append(assetLocation.SubresourcePath);
		//	pSubResource->m_ID = uuid;
		//	pSubResource->m_Name = meta.Name().empty() ? namePath.string() : meta.Name();
		//	m_pLoadedAssets.Set(uuid, pSubResource);
		//	m_pEngine->GetAssetDatabase().m_Callbacks.EnqueueCallback(CallbackType::CT_AssetLoaded, uuid, pSubResource);
		//	return pSubResource;
		//}

		LoaderModule* pModule = m_pEngine->GetLoaderModule(meta.Hash());

		if (pModule == nullptr) return nullptr;

		//if (assetLocation.IsSubAsset)
		//{
		//	throw new std::exception("Not implemented yet");
		//}

		std::filesystem::path path = m_pEngine->GetAssetDatabase().m_AssetPath;
		path.append(assetLocation.Path);

		if (!std::filesystem::exists(path))
			path = assetLocation.Path;

		Resource* pResource = pModule->Load(path.string());
		if (pResource == nullptr)
		{
			m_pEngine->GetDebug().LogError("Failed to load asset: " + std::to_string(uuid) + " at path: " + path.string());
			return nullptr;
		}

		std::filesystem::path namePath = assetLocation.Path;
		if (!assetLocation.SubresourcePath.empty()) namePath.append(assetLocation.SubresourcePath);
		pResource->m_ID = uuid;
		pResource->m_Name = meta.Name().empty() ? namePath.string() : meta.Name();
		m_pLoadedAssets.Set(uuid, pResource);
		m_pEngine->GetAssetDatabase().m_Callbacks.EnqueueCallback(CallbackType::CT_AssetLoaded, uuid, pResource);
		return pResource;
	}

	AssetManager::AssetManager(Engine* pEngine) : m_pEngine(pEngine), m_pResourceLoadingPool(nullptr) {}

	AssetManager::~AssetManager()
	{
		m_pEngine = nullptr;
	}

	void AssetManager::Initialize()
	{
		m_pEngine->GetAssetDatabase().Initialize();

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
		m_pEngine->GetAssetDatabase().Destroy();

		m_pResourceLoadingPool = nullptr;
	}

	void AssetManager::RunCallbacks()
	{
		m_pEngine->GetAssetDatabase().m_Callbacks.RunCallbacks();

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

	AssetManager::LoadingLock::LoadingLock(AssetManager* pManager, UUID uuid):
		m_pManager(pManager), m_UUID(uuid), IsValid(false)
	{
		if (pManager->m_pLoadingAssets.Contains(uuid)) return;
		pManager->m_pLoadingAssets.push_back(uuid);
		IsValid = true;
	}

	AssetManager::LoadingLock::~LoadingLock()
	{
		if (!IsValid) return;
		m_pManager->m_pLoadingAssets.Erase(m_UUID);
		IsValid = false;
	}
}
