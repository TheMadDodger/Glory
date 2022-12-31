#pragma once
#include "UUID.h"
#include "AssetLocation.h"
#include "ResourceMeta.h"
#include "AssetCallbacks.h"
#include "ThreadedVar.h"
#include <unordered_map>
#include "GScene.h"

#define ASSET_DATABASE Glory::GloryContext::GetAssetDatabase()
#define ASSET_MANAGER Glory::GloryContext::GetAssetManager()

namespace Glory
{
	class AssetDatabase
	{
	public:
		static bool GetAssetLocation(UUID uuid, AssetLocation& location);
		static bool GetResourceMeta(UUID uuid, ResourceMeta& meta);
		static UUID GetAssetUUID(const std::string& path);
		static bool AssetExists(UUID uuid);
		static bool AssetExists(const std::string& path);
		static void InsertAsset(const std::string& path, const ResourceMeta& meta, bool setDirty = true);
		static void UpdateAssetPath(UUID uuid, const std::string& newPath);
		static void UpdateAssetPath(UUID uuid, const std::string& newPath, const std::string& newMetaPath);
		static void UpdateAsset(UUID uuid, long lastSaved);
		static long GetLastSavedRecord(UUID uuid);
		static void UpdateAssetPaths(const std::string& oldPath, const std::string& newPath);
		static void DeleteAsset(UUID uuid);
		static void DeleteAssets(const std::string& path);
		static void IncrementAssetVersion(UUID uuid);
		static void Save();
		static void Load();

		static void CreateAsset(Resource* pResource, const std::string& path);
		static void ImportAsset(const std::string& path, Resource* pLoadedResource = nullptr);
		static void ImportNewScene(const std::string& path, GScene* pScene);
		static void SaveAsset(Resource* pResource, bool markUndirty = true);

		static void ForEachAssetLocation(std::function<void(UUID, const AssetLocation&)> callback);
		static void RemoveAsset(UUID uuid);

		static void GetAllAssetsOfType(size_t typeHash, std::vector<UUID>& out);
		static void GetAllAssetsOfType(size_t typeHash, std::vector<std::string>& out);

		static std::string GetAssetName(UUID uuid);

		static void SetAssetDirty(Object* pResource);
		static void SetAssetDirty(UUID uuid);
		static void SaveDirtyAssets();

		static void SetDirty(bool dirty = true);
		static bool IsDirty();

	private:
		static void Initialize();
		static void Destroy();
		static void Clear();
		static void ExportEditor(YAML::Emitter& out);
		static void ExportBuild(YAML::Emitter& out);

	private:
		friend class AssetManager;
		friend class GloryContext;
		friend class AssetCallbacks;
		AssetDatabase();
		virtual ~AssetDatabase();

	private:
		ThreadedUMap<UUID, AssetLocation> m_AssetLocations;
		ThreadedUMap<std::string, UUID> m_PathToUUID;
		ThreadedUMap<UUID, ResourceMeta> m_Metas;
		ThreadedUMap<size_t, std::vector<UUID>> m_AssetsByType;
		AssetCallbacks m_Callbacks;
		ThreadedVector<UUID> m_UnsavedAssets;
		ThreadedUMap<UUID, long> m_LastSavedRecords;
		bool m_IsDirty;
	};
}
