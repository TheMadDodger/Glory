#include <fstream>
#include <filesystem>
#include "AssetDatabase.h"
#include "LayerManager.h"

namespace Glory
{
	std::unordered_map<UUID, AssetLocation> AssetDatabase::m_AssetLocations;
	std::unordered_map<std::string, UUID> AssetDatabase::m_PathToUUID;
	std::unordered_map<UUID, ResourceMeta> AssetDatabase::m_Metas;
	std::unordered_map<size_t, std::vector<UUID>> AssetDatabase::m_AssetsByType;

	const AssetLocation* AssetDatabase::GetAssetLocation(UUID uuid)
	{
		if (m_AssetLocations.find(uuid) == m_AssetLocations.end())
		{
			//throw new std::exception("Asset not found!");
			return nullptr;
		}

		const AssetLocation* pLocation = &m_AssetLocations[uuid];
		return pLocation;
	}

	const ResourceMeta* AssetDatabase::GetResourceMeta(UUID uuid)
	{
		if (m_Metas.find(uuid) == m_Metas.end())
		{
			//throw new std::exception("Asset not found!");
			return nullptr;
		}

		const ResourceMeta* pMeta = &m_Metas[uuid];
		return pMeta;
	}

	UUID AssetDatabase::GetAssetUUID(const std::string& path)
	{
		if (m_PathToUUID.find(path) == m_PathToUUID.end()) return 0;
		return m_PathToUUID[path];
	}

	bool AssetDatabase::AssetExists(UUID uuid)
	{
		return m_AssetLocations.find(uuid) != m_AssetLocations.end();
	}

	bool AssetDatabase::AssetExists(const std::string& path)
	{
		return m_PathToUUID.find(path) != m_PathToUUID.end();
	}

	void AssetDatabase::InsertAsset(const std::string& path, const ResourceMeta& meta)
	{
		uint64_t uuid = meta.ID();
		m_Metas[uuid] = meta;
		m_AssetLocations[uuid] = AssetLocation(path, m_Metas[m_Metas.size() - 1]);
		m_PathToUUID[path] = uuid;
		m_AssetsByType[meta.Hash()].push_back(uuid);
	}

	void AssetDatabase::UpdateAssetPath(UUID uuid, const std::string& newPath, const std::string& newMetaPath)
	{
		if (m_AssetLocations.find(uuid) == m_AssetLocations.end())
		{
			return;
		}

		AssetLocation* pLocation = &m_AssetLocations[uuid];
		pLocation->m_Path = newPath;
		m_PathToUUID.erase(pLocation->m_Path);
		m_PathToUUID[pLocation->m_Path] = uuid;

		ResourceMeta* pMeta = &m_Metas[uuid];
		pMeta->m_Path = newMetaPath;
	}

	void AssetDatabase::Save()
	{
		LayerManager::Save();

		YAML::Emitter out;
		ExportEditor(out);

		std::filesystem::path databasePath = Game::GetAssetPath();
		databasePath = databasePath.parent_path();
		databasePath.append("Assets.db");

		std::ofstream outStream(databasePath);
		outStream << out.c_str();
		outStream.close();
	}

	void AssetDatabase::Load()
	{
		LayerManager::Load();

		std::filesystem::path databasePath = Game::GetAssetPath();
		databasePath = databasePath.parent_path();
		databasePath.append("Assets.db");

		if (!std::filesystem::exists(databasePath)) return;
		Clear();

		YAML::Node node = YAML::LoadFile(databasePath.string());
		for (size_t i = 0; i < node.size(); i++)
		{
			YAML::Node element = node[i];
			YAML::Node data;

			std::string path = "";
			YAML_READ(element, data, Path, path, std::string);
			bool isSubAsset = false;
			YAML_READ(element, data, IsSubAsset, isSubAsset, bool);
			size_t index = 0;
			YAML_READ(element, data, Index, index, size_t);
			size_t versionHash = 0;
			YAML_READ(element, data, VersionHash, versionHash, size_t);
			UUID uuid = 0;
			YAML_READ(element, data, UUID, uuid, uint64_t);
			size_t hash = 0;
			YAML_READ(element, data, Hash, hash, size_t);
			std::string extension = "";
			YAML_READ(element, data, Extension, extension, std::string);

			std::filesystem::path metaFilePath("./Assets" + path);
			std::filesystem::path metaExtension = std::filesystem::path(".gmeta");
			metaFilePath = metaFilePath.replace_extension(metaExtension);
			ResourceMeta meta(metaFilePath.string(), extension, uuid, hash);
			YAML::Node importSettingsNode = element["ImportSettings"];
			meta.m_ImportSettings = ImportSettings();
			LoaderModule* pLoader = Game::GetGame().GetEngine()->GetLoaderModule(hash);
			if (pLoader)
			{
				if (meta.Exists())
				{
					size_t originalVersionHash = std::hash<ResourceMeta>()(meta);
					if (originalVersionHash != versionHash)
						meta.Read();
					else
						meta.m_ImportSettings = pLoader->ReadImportSettings(importSettingsNode);
				}
				else meta.m_ImportSettings = pLoader->ReadImportSettings(importSettingsNode);
			}

			InsertAsset(path, meta);
		}
	}

	void AssetDatabase::ForEachAssetLocation(std::function<void(UUID, const AssetLocation&)> callback)
	{
		for (auto it = m_AssetLocations.begin(); it != m_AssetLocations.end(); it++)
		{
			callback(it->first, it->second);
		}
	}

	void AssetDatabase::RemoveAsset(UUID uuid)
	{
		if (m_AssetLocations.find(uuid) == m_AssetLocations.end())
			return;

		AssetLocation* pLocation = &m_AssetLocations[uuid];
		std::string path = pLocation->m_Path;
		m_AssetLocations.erase(uuid);
		m_Metas.erase(uuid);
		m_PathToUUID.erase(path);
	}

	const std::vector<UUID>& AssetDatabase::GetAllAssetsOfType(size_t typeHash)
	{
		if (m_AssetsByType.find(typeHash) == m_AssetsByType.end()) return std::vector<UUID>();
		return m_AssetsByType[typeHash];
	}

	std::string AssetDatabase::GetAssetName(UUID uuid)
	{
		// Will need to be different for build and editor since assets are grouped together in a single file when built
		const AssetLocation* pLocation = GetAssetLocation(uuid);
		std::filesystem::path path(pLocation->m_Path);
		return path.filename().replace_extension("").string();
	}

	void AssetDatabase::Initialize()
	{
		//Load();
	}

	void AssetDatabase::Destroy()
	{
		Clear();
	}

	void AssetDatabase::Clear()
	{
		m_AssetLocations.clear();
		m_PathToUUID.clear();
		m_Metas.clear();
	}

	void AssetDatabase::ExportEditor(YAML::Emitter& out)
	{
		out << YAML::BeginSeq;
		for (auto it = m_AssetLocations.begin(); it != m_AssetLocations.end(); it++)
		{
			UUID uuid = it->first;
			AssetLocation location = it->second;

			if (m_Metas.find(uuid) == m_Metas.end())
				continue;

			const ResourceMeta& meta = m_Metas.at(uuid);

			size_t versionHash = std::hash<ResourceMeta>()(meta);

			out << YAML::BeginMap;
			out << YAML::Key << "Path";
			out << YAML::Value << location.m_Path;
			out << YAML::Key << "IsSubAsset";
			out << YAML::Value << location.m_IsSubAsset;
			out << YAML::Key << "Index";
			out << YAML::Value << location.m_Index;
			out << YAML::Key << "VersionHash";
			out << YAML::Value << versionHash;
			size_t hash = meta.Hash();
			LoaderModule* pLoader = Game::GetGame().GetEngine()->GetLoaderModule(hash);
			meta.Write(out, pLoader);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
	}

	void AssetDatabase::ExportBuild(YAML::Emitter& out)
	{
		
	}

	AssetDatabase::AssetDatabase() {}

	AssetDatabase::~AssetDatabase() {}
}
