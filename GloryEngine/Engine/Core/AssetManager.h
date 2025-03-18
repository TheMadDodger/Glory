#pragma once
#include "UUID.h"
#include "Resource.h"

#include <unordered_map>
#include <vector>
#include <functional>

namespace Glory
{
	class Engine;

	/** @brief Base class for asset management */
	class AssetManager
	{
	public:
		/** @brief Constructor */
		AssetManager(Engine* pEngine);
		/** @brief Destructor */
		virtual ~AssetManager();

		/**
		 * @brief Get or load an asset
		 * @param T Type of the asset to load
		 * @param uuid ID of the asset to load
		 *
		 * If the asset is not loaded, it gets loaded asynchronously.
		 */
		template<class T>
		T* GetOrLoadAsset(UUID uuid)
		{
			Resource* pResource = GetOrLoadAsset(uuid);
			if (!pResource) return nullptr;
			return (T*)pResource;
		}

		/** @overload */
		virtual Resource* GetOrLoadAsset(UUID uuid) = 0;

		/**
		 * @brief Get or load an asset
		 * @param T Type of the asset to load
		 * @param uuid ID of the asset to load
		 *
		 * If the asset is not loaded, it gets loaded synchronously.
		 */
		template<class T>
		T* GetAssetImmediate(UUID uuid)
		{
			Resource* pResource = GetAssetImmediate(uuid);
			if (!pResource) return nullptr;
			return (T*)pResource;
		}

		/** @overload */
		virtual Resource* GetAssetImmediate(UUID uuid) = 0;

		/**
		 * @brief Get or load an assdet by callback
		 * @param uuid ID of the asset
		 * @param callback Callback to call when the asset is loaded
		 *
		 * If the asset is not loaded, it gets loaded asynchronously.
		 */
		virtual void GetAsset(UUID uuid, std::function<void(Resource*)> callback) = 0;
		/**
		 * @brief Unload an asset by deleting it from memory
		 * @param uuid ID of the asset to unload
		 */
		virtual void UnloadAsset(UUID uuid) = 0;
		/**
		 * @brief Find a loaded asset by ID
		 * @param uuid ID of the asset
		 */
		virtual Resource* FindResource(UUID uuid) = 0;
		/**
		 * @brief Add a loaded asset to the manager
		 * @param pResource Loaded asset to add
		 * @param uuid ID of the asset
		 */
		virtual void AddLoadedResource(Resource* pResource, UUID uuid) = 0;
		/** @overload */
		virtual void AddLoadedResource(Resource* pResource) = 0;

		/** @brief Initialize the manager */
		virtual void Initialize() = 0;

	protected:
		Engine* m_pEngine;

	private:
		friend class Engine;
		friend class AssetDatabase;
	};
}
