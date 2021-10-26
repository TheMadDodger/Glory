#pragma once
#include "UUID.h"
#include "Resource.h"
#include "AssetGroup.h"
#include "Game.h"
#include "AssetDatabase.h"
#include <unordered_map>
#include <vector>

namespace Glory
{
	class AssetManager
	{
	public:
		template<class T>
		static T* GetAsset(UUID uuid)
		{
			Resource* pResource = FindResource(uuid);
			if (pResource) return static_cast<T*>(pResource);

			const AssetLocation* pAssetLocation = AssetDatabase::GetAssetLocation(uuid);
			if (!pAssetLocation) return nullptr;
			LoaderModule* pModule = Game::GetGame().GetEngine()->GetLoaderModule<T>();

			if (pAssetLocation->m_IsSubAsset)
			{
				throw new std::exception("Not implemented yet");
			}

			pResource = pModule->Load(pAssetLocation->m_Path);
			
			m_pLoadedAssets[uuid] = pResource;
			return pResource;
		}

	private:
		static Resource* FindResource(UUID uuid);

	private:
		AssetManager();
		virtual ~AssetManager();

		static void Initialize();
		static void Destroy();

	private:
		friend class Engine;
		static std::unordered_map<UUID, Resource*> m_pLoadedAssets;
		static std::vector<AssetGroup*> m_LoadedAssetGroups;
		static std::unordered_map<std::string, size_t> m_PathToGroupIndex;
	};
}
