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
		static bool AssetExists(UUID uuid);
		static void InsertAsset(const std::string& path, const ResourceMeta& meta);

	private:
		static void Initialize();
		static void Destroy();
		static void ExportEditor(std::basic_ostream<char, std::char_traits<char>>& ostream);
		static void ExportBuild(std::basic_ostream<char, std::char_traits<char>>& ostream);

	private:
		friend class AssetManager;
		AssetDatabase();
		virtual ~AssetDatabase();

	private:
		static std::unordered_map<UUID, AssetLocation> m_AssetLocations;
		static std::vector<ResourceMeta> m_Metas;
	};
}
