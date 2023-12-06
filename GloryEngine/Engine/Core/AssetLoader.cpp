#include "AssetLoader.h"
#include "Engine.h"

#include "AssetDatabase.h"
#include "BinaryStream.h"

namespace Glory
{
	AssetLoader::AssetLoader(Engine* pEngine):
		m_pEngine(pEngine), m_pLoadJobs(m_pEngine->Jobs().Run<bool, const std::filesystem::path>(2))
	{
	}

	AssetLoader::~AssetLoader()
	{
	}

	bool AssetLoader::RequestLoad(UUID uuid)
	{
		AssetLocation assetLocation;
		if (!AssetDatabase::GetAssetLocation(uuid, assetLocation)) return false;

		std::filesystem::path path = Game::GetAssetPath();
		path.append(assetLocation.Path);
		if (path.extension().compare(".gcag") != 0) return false;

		const auto itor = std::find(m_LoadingPaths.begin(), m_LoadingPaths.end(), path);
		/* Already loading, but doesn't mean loading failed */
		if (itor != m_LoadingPaths.end()) return true;

		m_pLoadJobs->QueueSingleJob(std::bind(&AssetLoader::LoadJob, this, std::placeholders::_1), path);
	}

	void AssetLoader::DumpLoadedArchives()
	{
		if (m_LoadedArchives.Size())
		{
			m_LoadedArchives.ForEachClear([this](const AssetArchive& archive) {
				if (!archive) return;
				for (size_t i = 0; i < archive.Size(); ++i)
				{
					Resource* pResource = archive.Get(i);
					if (m_pEngine->GetResources().Add(pResource)) continue;
					delete pResource;
				}
			});
		}
	}

	bool AssetLoader::LoadJob(const std::filesystem::path path)
	{
		AssetArchive archive = LoadArchive(path);
		m_LoadedArchives.push_back(std::move(archive));
		return true;
	}

	AssetArchive AssetLoader::LoadArchive(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			std::stringstream str;
			str << "Failed to load asset archive at path " << path << " file not found!";
			Debug::LogError(str.str());
			return AssetArchive{};
		}

		/* Load as archive */
		BinaryFileStream stream{ path };
		AssetArchive archive{ &stream };
		archive.Deserialize();
		return archive;
	}

	/*void AssetManager::GetAsset(UUID uuid, std::function<void(Resource*)> callback)
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
		else if (callback != NULL) ASSET_MANAGER->m_AssetLoadedCallbacks.Set(uuid, { callback });

		ASSET_MANAGER->m_pResourceLoadingPool->QueueSingleJob(AssetManager::LoadResourceJob, uuid);
	}*/

	/*void AssetManager::RunCallbacks()
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
	}*/
}
