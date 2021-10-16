#include "AssetManager.h"
#include "AssetDatabase.h"

namespace Glory
{
	std::unordered_map<UUID, Resource*> AssetManager::m_pLoadedAssets;
	std::vector<AssetGroup*> AssetManager::m_LoadedAssetGroups;
	std::unordered_map<std::string, size_t> AssetManager::m_PathToGroupIndex;

	Resource* AssetManager::FindResource(UUID uuid)
	{
		auto it = m_pLoadedAssets.find(uuid);
		if (it == m_pLoadedAssets.end()) return nullptr;
		return m_pLoadedAssets[uuid];
	}

	AssetManager::AssetManager() {}

	AssetManager::~AssetManager() {}

	void AssetManager::Initialize()
	{
		AssetDatabase::Initialize();
	}

	void AssetManager::Destroy()
	{
		for (auto it = m_pLoadedAssets.begin(); it != m_pLoadedAssets.end(); ++it)
		{
			delete it->second;
		}

		for (size_t i = 0; i < m_LoadedAssetGroups.size(); ++i)
		{
			delete m_LoadedAssetGroups[i];
		}

		m_pLoadedAssets.clear();
		m_LoadedAssetGroups.clear();
		m_PathToGroupIndex.clear();
		AssetDatabase::Destroy();
	}
}
