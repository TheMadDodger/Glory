#pragma once
#include "GloryEditor.h"

#include <AssetManager.h>
#include <ThreadedVar.h>

namespace Glory
{
	class Engine;
	class Resource;
	class AssetArchive;
}

namespace Glory::Editor
{
	class EditorApplication;

	struct CallbackData
	{
		CallbackData();
		CallbackData(UUID uuid, Resource* pResource);

		UUID m_UUID;
		Resource* m_pResource;
	};

	class EditorAssetManager : public AssetManager
	{
	public:
		EditorAssetManager(EditorApplication* pApplication);
		virtual ~EditorAssetManager();

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

		GLORY_EDITOR_API Resource* GetAssetImmediate(UUID uuid);
		GLORY_EDITOR_API void GetAsset(UUID uuid, std::function<void(Resource*)> callback) override;
		GLORY_EDITOR_API Resource* GetOrLoadAsset(UUID uuid) override;
		GLORY_EDITOR_API void UnloadAsset(UUID uuid) override;
		GLORY_EDITOR_API Resource* FindResource(UUID uuid) override;
		GLORY_EDITOR_API void AddLoadedResource(Resource* pResource, UUID uuid) override;
		GLORY_EDITOR_API void AddLoadedResource(Resource* pResource) override;

		GLORY_EDITOR_API void ReloadAsset(UUID uuid);

		GLORY_EDITOR_API bool IsLoading(UUID uuid);
		GLORY_EDITOR_API void GetAllLoading(std::vector<UUID>& out);

		//const AssetArchive* GetOrLoadArchive(const std::filesystem::path& path);
		//void AddAssetArchive(uint32_t hash, AssetArchive&& archive);

	private:
		bool LoadResourceJob(UUID uuid);
		Resource* LoadAsset(UUID uuid);

	private:
		void Initialize() override;
		void Destroy() override;
		void RunCallbacks();

	private:
		friend class EditorApplication;
		friend class AssetDatabase;

		struct LoadingLock
		{
			LoadingLock(EditorAssetManager* pManager, UUID uuid);
			~LoadingLock();

			EditorAssetManager* m_pManager;
			bool IsValid;
			UUID m_UUID;
		};

		Engine* m_pEngine;
		//ThreadedUMap<uint32_t, AssetArchive> m_LoadedArchives;
		ThreadedUMap<UUID, Resource*> m_pLoadedAssets;
		ThreadedVector<UUID> m_pLoadingAssets;
		ThreadedUMap<std::string, size_t> m_PathToGroupIndex;
		ThreadedQueue<CallbackData> m_ResourceLoadedCallbacks;
		ThreadedUMap<UUID, std::vector<std::function<void(Resource*)>>> m_AssetLoadedCallbacks;
		Jobs::JobPool<bool, UUID>* m_pResourceLoadingPool;
	};
}
