#pragma once
#include "GloryEditor.h"

#include <AssetManager.h>
#include <ThreadedVar.h>
#include <JobManager.h>

namespace Glory
{
	class Engine;
	class Resource;
	class AssetArchive;
}

namespace Glory::Editor
{
	class EditorApplication;

	/** @brief Callback data for when a resource is loaded */
	struct CallbackData
	{
		/** @brief Constructor */
		CallbackData();
		/** @brief Destructor */
		CallbackData(UUID uuid, Resource* pResource);

		UUID m_UUID;
		Resource* m_pResource;
	};

	/** @brief Editor asset mamager */
	class EditorAssetManager : public AssetManager
	{
	public:
		/** @brief Base class for asset management */
		EditorAssetManager(EditorApplication* pApplication);
		/** @brief Destructor */
		virtual ~EditorAssetManager();

		/**
		 * @brief Get or load an asset
		 * @param uuid ID of the asset to load
		 *
		 * If the asset is not loaded, it gets loaded synchronously.
		 */
		GLORY_EDITOR_API Resource* GetAssetImmediate(UUID uuid);
		/**
		 * @brief Get or load an assdet by callback
		 * @param uuid ID of the asset
		 * @param callback Callback to call when the asset is loaded
		 *
		 * If the asset is not loaded, it gets loaded asynchronously.
		 */
		GLORY_EDITOR_API void GetAsset(UUID uuid, std::function<void(Resource*)> callback) override;
		/**
		 * @brief Get or load an asset
		 * @param uuid ID of the asset to load
		 *
		 * If the asset is not loaded, it gets loaded asynchronously.
		 */
		GLORY_EDITOR_API Resource* GetOrLoadAsset(UUID uuid) override;
		/**
		 * @brief Unload an asset by deleting it from memory
		 * @param uuid ID of the asset to unload
		 */
		GLORY_EDITOR_API void UnloadAsset(UUID uuid) override;
		/**
		 * @brief Find a loaded asset by ID
		 * @param uuid ID of the asset
		 */
		GLORY_EDITOR_API Resource* FindResource(UUID uuid) override;
		/**
		 * @brief Add a loaded asset to the manager
		 * @param pResource Loaded asset to add
		 * @param uuid ID of the asset
		 */
		GLORY_EDITOR_API void AddLoadedResource(Resource* pResource, UUID uuid) override;
		/** @overload */
		GLORY_EDITOR_API void AddLoadedResource(Resource* pResource) override;

		/**
		 * @brief Reload an asset
		 * @param uuid ID of the asset
		 */
		GLORY_EDITOR_API void ReloadAsset(UUID uuid);

		/**
		 * @brief Check whether an asset is loading
		 * @param uuid ID of the asset
		 */
		GLORY_EDITOR_API bool IsLoading(UUID uuid);
		/**
		 * @brief Get all currently loading assets
		 * @param out Vector to write IDs to
		 */
		GLORY_EDITOR_API void GetAllLoading(std::vector<UUID>& out);

	private:
		/**
		 * @brief Job callback for loading a resource
		 * @param uuid ID of the asset
		 */
		bool LoadResourceJob(UUID uuid);
		/**
		 * @brief Helper function for loading an asset
		 * @param uuid ID of the asset to load
		 *
		 * Currently waits for the asset to be compiled by the @ref AssetCompiler
		 */
		Resource* LoadAsset(UUID uuid);

	private:
		/** @brief Initialize the manager */
		void Initialize() override;
		/** @brief Trigger and clear callbacks */
		void RunCallbacks();

	private:
		friend class EditorApplication;
		friend class AssetDatabase;

		/** @brief Loading lock for thread safety */
		struct LoadingLock
		{
			/** @brief Constructor */
			LoadingLock(EditorAssetManager* pManager, UUID uuid);
			/** @brief Destructor */
			~LoadingLock();

			EditorAssetManager* m_pManager;
			bool IsValid;
			UUID m_UUID;
		};

		ThreadedUMap<UUID, Resource*> m_pLoadedAssets;
		ThreadedVector<UUID> m_pLoadingAssets;
		ThreadedQueue<CallbackData> m_ResourceLoadedCallbacks;
		ThreadedUMap<UUID, std::vector<std::function<void(Resource*)>>> m_AssetLoadedCallbacks;
		Jobs::JobWorkerPool<bool, UUID>* m_pResourceLoadingPool;
	};
}
