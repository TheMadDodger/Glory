#pragma once
#include "UUID.h"
#include "AssetLocation.h"
#include "ResourceMeta.h"
#include <unordered_map>

namespace Glory
{
	class AssetDatabase
	{
	public:
		static const AssetLocation* GetAssetLocation(UUID uuid);
		static const ResourceMeta* GetResourceMeta(UUID uuid);
		static UUID GetAssetUUID(const std::string& path);
		static bool AssetExists(UUID uuid);
		static bool AssetExists(const std::string& path);
		static void InsertAsset(const std::string& path, const ResourceMeta& meta);
		static void UpdateAssetPath(UUID uuid, const std::string& newPath, const std::string& newMetaPath);
		static void Save();
		static void Load();

		static void ForEachAssetLocation(std::function<void(UUID, const AssetLocation&)> callback);
		static void RemoveAsset(UUID uuid);

	private:
		static void Initialize();
		static void Destroy();
		static void ExportEditor(YAML::Emitter& out);
		static void ExportBuild(YAML::Emitter& out);

	private:
		friend class AssetManager;
		AssetDatabase();
		virtual ~AssetDatabase();

	private:
		static std::unordered_map<UUID, AssetLocation> m_AssetLocations;
		static std::unordered_map<std::string, UUID> m_PathToUUID;
		static std::unordered_map<UUID, ResourceMeta> m_Metas;
	};
}
