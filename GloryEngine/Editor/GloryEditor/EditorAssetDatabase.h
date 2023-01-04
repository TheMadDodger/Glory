#pragma once
#include "ThreadedVar.h"
#include "UUID.h"
#include "GloryEditor.h"
#include <yaml-cpp/yaml.h>
#include <ResourceMeta.h>
#include <AssetLocation.h>
#include <JobManager.h>

namespace Glory
{
	class GScene;
}

namespace Glory::Editor
{
	class EditorAssetDatabase
	{
	public:
		static GLORY_EDITOR_API void Load(YAML::Node& projectNode);
		static GLORY_EDITOR_API void Reload();
		static GLORY_EDITOR_API void InsertAsset(AssetLocation& location, const ResourceMeta& meta, bool setDirty = true);
		static GLORY_EDITOR_API void UpdateAssetPath(UUID uuid, const std::string& newPath);
		static GLORY_EDITOR_API void UpdateAsset(UUID uuid, long lastSaved);
		static GLORY_EDITOR_API long GetLastSavedRecord(UUID uuid);
		static GLORY_EDITOR_API void UpdateAssetPaths(const std::string& oldPath, const std::string& newPath);
		static GLORY_EDITOR_API void DeleteAsset(UUID uuid);
		static GLORY_EDITOR_API void DeleteAsset(const std::string& path);
		static GLORY_EDITOR_API void DeleteAssets(const std::string& path);
		static GLORY_EDITOR_API void IncrementAssetVersion(UUID uuid);
		static GLORY_EDITOR_API void CreateAsset(Resource* pResource, const std::string& path);
		static GLORY_EDITOR_API void ImportAsset(const std::string& path, Resource* pLoadedResource = nullptr, std::filesystem::path subPath = "");
		static GLORY_EDITOR_API void ImportAssetsAsync(const std::string& path);
		static GLORY_EDITOR_API void ImportAssetAsync(const std::string& path);
		static GLORY_EDITOR_API void ImportNewScene(const std::string& path, GScene* pScene);
		static GLORY_EDITOR_API void SaveAsset(Resource* pResource, bool markUndirty = true);
		static GLORY_EDITOR_API void RemoveAsset(UUID uuid);
		static GLORY_EDITOR_API void SetAssetDirty(Object* pResource);
		static GLORY_EDITOR_API void SetAssetDirty(UUID uuid);
		static GLORY_EDITOR_API void SaveDirtyAssets();
		static GLORY_EDITOR_API void SetDirty(bool dirty = true);
		static GLORY_EDITOR_API bool IsDirty();

		static GLORY_EDITOR_API bool GetAssetLocation(UUID uuid, AssetLocation& location);
		static GLORY_EDITOR_API bool GetAssetMetadata(UUID uuid, ResourceMeta& meta);

		static GLORY_EDITOR_API UUID FindAssetUUID(const std::string& path);
		static GLORY_EDITOR_API bool AssetExists(UUID uuid);
		static GLORY_EDITOR_API std::string GetAssetName(UUID uuid);
		static GLORY_EDITOR_API void GetAllAssetsOfType(size_t typeHash, std::vector<UUID>& result);

		static GLORY_EDITOR_API void RegisterAsyncImportCallback(std::function<void(Resource*)> func);

		static GLORY_EDITOR_API void ImportModuleAssets();

	private:
		static void Initialize();
		static void Cleanup();
		static void Update();

		static bool ImportJob(std::filesystem::path path);
		static void ImportModuleAssets(const std::filesystem::path& path);

	private:
		friend class MainEditor;
		static YAML::Node m_DatabaseNode;

		static ThreadedVector<UUID> m_UnsavedAssets;

		struct ImportedResource
		{
			Resource* Resource;
			std::filesystem::path Path;
		};
		static ThreadedVector<ImportedResource> m_ImportedResources;
		static ThreadedUMap<UUID, long> m_LastSavedRecords;
		static bool m_IsDirty;

		static std::function<void(Resource*)> m_AsyncImportCallback;

		static Jobs::JobPool<bool, std::filesystem::path>* m_pImportPool;

	private:
		EditorAssetDatabase() = delete;
	};
}
