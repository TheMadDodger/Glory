#pragma once
#include "UUID.h"
#include "AssetLocation.h"
#include "ResourceMeta.h"
#include "AssetCallbacks.h"
#include "ThreadedVar.h"
#include <unordered_map>

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
		static void InsertAsset(const std::string& path, const ResourceMeta& meta);
		static void UpdateAssetPath(UUID uuid, const std::string& newPath, const std::string& newMetaPath);
		static void Save();
		static void Load();

		static void CreateAsset(Resource* pResource, const std::string& path);
		static void ImportAsset(const std::string& path, Resource* pLoadedResource = nullptr);

		static void ForEachAssetLocation(std::function<void(UUID, const AssetLocation&)> callback);
		static void RemoveAsset(UUID uuid);

		static const std::vector<UUID> GetAllAssetsOfType(size_t typeHash);

		static std::string GetAssetName(UUID uuid);

	private:
		static void Initialize();
		static void Destroy();
		static void Clear();
		static void ExportEditor(YAML::Emitter& out);
		static void ExportBuild(YAML::Emitter& out);

	private:
		friend class AssetManager;
		AssetDatabase();
		virtual ~AssetDatabase();

	private:
		static ThreadedUMap<UUID, AssetLocation> m_AssetLocations;
		static ThreadedUMap<std::string, UUID> m_PathToUUID;
		static ThreadedUMap<UUID, ResourceMeta> m_Metas;
		static ThreadedUMap<size_t, std::vector<UUID>> m_AssetsByType;
		static AssetCallbacks m_Callbacks;
	};
}
