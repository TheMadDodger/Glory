#pragma once
#include "UUID.h"
#include "AssetLocation.h"
#include "ResourceMeta.h"
#include "AssetCallbacks.h"
#include "GScene.h"
#include <map>

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
		static void EnqueueCallback(const CallbackType& type, UUID uuid, Resource* pResource);

		static void GetAllAssetsOfType(size_t typeHash, std::vector<UUID>& out);
		static void GetAllAssetsOfType(size_t typeHash, std::vector<std::string>& out);

		static std::string GetAssetName(UUID uuid);

		static void Load(YAML::Node& node);

		static void Clear();

		static void SetIDAndName(Resource* pResource, UUID newID, const std::string& name);

	private:
		static void Initialize();
		static void Destroy();

		static void SetAsset(AssetLocation& assetLocation, const ResourceMeta& meta);
		//static void ExportEditor(YAML::Emitter& out);
		//static void ExportBuild(YAML::Emitter& out);

	private:
		friend class AssetManager;
		friend class GloryContext;
		friend class AssetCallbacks;
		AssetDatabase();
		virtual ~AssetDatabase();

	private:
		bool m_Initialized;
		bool m_IsReading;
		std::map<UUID, AssetLocation> m_AssetLocations;
		std::map<std::string, UUID> m_PathToUUID;
		std::map<UUID, ResourceMeta> m_Metas;
		std::map<size_t, std::vector<UUID>> m_AssetsByType;
		AssetCallbacks m_Callbacks;

		struct ReadLock
		{
		public:
			ReadLock();
			~ReadLock();

		private:
			static size_t m_LockCounter;
		};
	};
}
