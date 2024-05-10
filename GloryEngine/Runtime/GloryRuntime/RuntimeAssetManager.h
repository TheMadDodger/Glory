#pragma once
#include <AssetManager.h>

namespace Glory
{
	class AssetArchive;

	/** @brief Runtime asset manager */
	class RuntimeAssetManager : public AssetManager
	{
	public:
		/** @brief Constructor */
		RuntimeAssetManager(Engine* pEngine);
		/** @brief Destructor */
		virtual ~RuntimeAssetManager();

		/**
		 * @brief Get or load an asset
		 * @param uuid ID of the asset to load
		 *
		 * If the asset is not loaded, it gets loaded synchronously.
		 */
		Resource* GetAssetImmediate(UUID uuid);
		/**
		 * @brief Get or load an assdet by callback
		 * @param uuid ID of the asset
		 * @param callback Callback to call when the asset is loaded
		 *
		 * If the asset is not loaded, it gets loaded asynchronously.
		 */
		void GetAsset(UUID uuid, std::function<void(Resource*)> callback) override;
		/**
		 * @brief Get or load an asset
		 * @param uuid ID of the asset to load
		 *
		 * If the asset is not loaded, it gets loaded asynchronously.
		 */
		Resource* GetOrLoadAsset(UUID uuid) override;
		/**
		 * @brief Unload an asset by deleting it from memory
		 * @param uuid ID of the asset to unload
		 */
		void UnloadAsset(UUID uuid) override;
		/**
		 * @brief Find a loaded asset by ID
		 * @param uuid ID of the asset
		 */
		Resource* FindResource(UUID uuid) override;
		/**
		 * @brief Add a loaded asset to the manager
		 * @param pResource Loaded asset to add
		 * @param uuid ID of the asset
		 */
		void AddLoadedResource(Resource* pResource, UUID uuid) override;
		/** @overload */
		void AddLoadedResource(Resource* pResource) override;

		/**
		 * @brief Get or load an asset archive
		 * @param path Path to asset archive to load
		 */
		const AssetArchive* GetOrLoadArchive(const std::filesystem::path& path);
		/**
		 * @brief Add a loaded asset archive
		 * @param hash Unique hash for the archive, hashed from the path
		 * @param archive Archive to add
		 */
		void AddAssetArchive(uint32_t hash, AssetArchive&& archive);

	private:
		/** @brief Initialize the manager */
		void Initialize() override;

	private:
		friend class EditorApplication;
		friend class AssetDatabase;

		ThreadedUMap<uint32_t, AssetArchive> m_LoadedArchives;
		ThreadedUMap<UUID, Resource*> m_pLoadedAssets;
	};
}
