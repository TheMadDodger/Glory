#include <fstream>
#include <filesystem>
#include "AssetDatabase.h"
#include "AssetManager.h"
#include "LayerManager.h"
#include "Engine.h"

namespace Glory
{
	bool AssetDatabase::GetAssetLocation(UUID uuid, AssetLocation& location)
	{
		ReadLock readLock{ this };
		if (m_AssetLocations.find(uuid) == m_AssetLocations.end())
		{
			return false;
		}

		location = m_AssetLocations[uuid];
		return true;
	}

	bool AssetDatabase::GetResourceMeta(UUID uuid, ResourceMeta& meta)
	{
		ReadLock readLock{ this };
		if (m_Metas.find(uuid) == m_Metas.end())
		{
			return false;
		}

		meta = m_Metas.at(uuid);
		return true;
	}

	UUID AssetDatabase::GetAssetUUID(const std::string& path)
	{
		ReadLock readLock{ this };
		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');
		UUID uuid = m_PathToUUID[fixedPath];
		return uuid;
	}

	bool AssetDatabase::AssetExists(UUID uuid)
	{
		ReadLock readLock{ this };
		return m_AssetLocations.find(uuid) != m_AssetLocations.end();
	}

	bool AssetDatabase::AssetExists(const std::string& path)
	{
		ReadLock readLock{ this };

		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');
		return m_PathToUUID.find(fixedPath) != m_PathToUUID.end();
	}

	void AssetDatabase::EnqueueCallback(const CallbackType& type, UUID uuid, Resource* pResource)
	{
		m_Callbacks.EnqueueCallback(type, uuid, pResource);
	}

	void AssetDatabase::SetAsset(AssetLocation& assetLocation, const ResourceMeta& meta)
	{
		std::replace(assetLocation.Path.begin(), assetLocation.Path.end(), '/', '\\');

		const uint64_t uuid = meta.ID();
		m_Metas[uuid] = meta;
		m_AssetLocations[uuid] = assetLocation;
		if (assetLocation.SubresourcePath.empty()) m_PathToUUID[assetLocation.Path] = uuid;
		m_AssetsByType[meta.m_TypeHash].push_back(uuid);
	}

	void AssetDatabase::Remove(UUID uuid)
	{
		const ResourceMeta& meta = m_Metas[uuid];
		const AssetLocation& location = m_AssetLocations[uuid];

		if (location.SubresourcePath.empty()) m_PathToUUID.erase(location.Path);
		std::remove(m_AssetsByType[meta.m_TypeHash].begin(), m_AssetsByType[meta.m_TypeHash].end(), uuid);
		m_AssetLocations.erase(uuid);
		m_Metas.erase(uuid);
	}

	void AssetDatabase::SetAssetPath(const std::filesystem::path& path)
	{
		m_AssetPath = path.string();
	}

	void AssetDatabase::SetSettingsPath(const std::filesystem::path& path)
	{
		m_SettingsPath = path.string();
	}

	void AssetDatabase::GetAllAssetsOfType(uint32_t typeHash, std::vector<UUID>& out)
	{
		ReadLock readLock{ this };

		if (m_AssetsByType.find(typeHash) == m_AssetsByType.end()) return;
		size_t size = out.size();
		out.resize(size + m_AssetsByType[typeHash].size());
		size_t copySize = sizeof(UUID) * m_AssetsByType[typeHash].size();
		memcpy(&out[size], &m_AssetsByType[typeHash][0], copySize);
	}

	void AssetDatabase::GetAllAssetsOfType(uint32_t typeHash, std::vector<std::string>& out)
	{
		ReadLock readLock{ this };

		if (m_AssetsByType.find(typeHash) == m_AssetsByType.end()) return;
		size_t size = out.size();
		out.resize(size + m_AssetsByType.at(typeHash).size());
		for (size_t i = 0; i < m_AssetsByType.at(typeHash).size(); i++)
		{
			UUID uuid = m_AssetsByType.at(typeHash)[i];
			AssetLocation location;
			GetAssetLocation(uuid, location);
			out.push_back(location.Path);
		}
	}

	std::string AssetDatabase::GetAssetName(UUID uuid)
	{
		ReadLock readLock{ this };

		// Will need to be different for build and editor since assets are grouped together in a single file when built
		AssetLocation location;
		if (!GetAssetLocation(uuid, location)) return "";
		std::filesystem::path path(location.Path);
		return path.filename().replace_extension("").string();
	}

	void AssetDatabase::SetEntryScene(UUID uuid)
	{
		m_EntrySceneID = uuid;
	}

	UUID AssetDatabase::GetEntryScene() const
	{
		return m_EntrySceneID;
	}

	void AssetDatabase::Initialize()
	{
		m_Callbacks.Initialize();
	}

	void AssetDatabase::Destroy()
	{
		m_Callbacks.Cleanup();
		Clear();
	}

	void AssetDatabase::Clear()
	{
		while (m_IsReading)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		m_IsWriting = false;
		m_AssetLocations.clear();
		m_PathToUUID.clear();
		m_Metas.clear();
		m_AssetsByType.clear();
	}

	void AssetDatabase::SetIDAndName(Resource* pResource, UUID newID, const std::string& name)
	{
		pResource->m_ID = newID;
		pResource->m_Name = name;
	}

	//void AssetDatabase::ExportEditor(YAML::Emitter& out)
	//{
	//	out << YAML::BeginSeq;
	//
	//	m_AssetLocations.ForEach([&](const UUID& uuid, const AssetLocation& location)
	//	{
	//		if (!m_Metas.Contains(uuid))
	//			return;
	//
	//		ResourceMeta meta = m_Metas[uuid];
	//
	//		size_t versionHash = std::hash<ResourceMeta>()(meta);
	//		long lastSaved = m_LastSavedRecords[uuid];
	//
	//		out << YAML::BeginMap;
	//		out << YAML::Key << "Path";
	//		out << YAML::Value << location.m_Path;
	//		out << YAML::Key << "IsSubAsset";
	//		out << YAML::Value << location.m_IsSubAsset;
	//		out << YAML::Key << "Index";
	//		out << YAML::Value << location.m_Index;
	//		out << YAML::Key << "VersionHash";
	//		out << YAML::Value << versionHash;
	//		out << YAML::Key << "LastSaved";
	//		out << YAML::Value << lastSaved;
	//		uint32_t hash = meta.Hash();
	//		LoaderModule* pLoader = Game::GetGame().GetEngine()->GetLoaderModule(hash);
	//		meta.Write(out, pLoader);
	//		out << YAML::EndMap;
	//	});
	//
	//	out << YAML::EndSeq;
	//}

	AssetDatabase::AssetDatabase()
		: m_IsWriting(false), m_IsReading(false), m_Callbacks(this) {}

	AssetDatabase::~AssetDatabase() {}

	size_t AssetDatabase::ReadLock::m_LockCounter = 0;
	size_t AssetDatabase::WriteLock::m_LockCounter = 0;

	AssetDatabase::ReadLock::ReadLock(AssetDatabase* pDatabase): m_pDatabase(pDatabase)
	{
		++m_LockCounter;
		while (pDatabase->m_IsWriting)
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		pDatabase->m_IsReading = true;
	}

	AssetDatabase::ReadLock::~ReadLock()
	{
		--m_LockCounter;
		if (m_LockCounter > 0) return;
		m_pDatabase->m_IsReading = false;
	}

	AssetDatabase::WriteLock::WriteLock(AssetDatabase* pDatabase): m_pDatabase(pDatabase)
	{
		++m_LockCounter;
		while (pDatabase->m_IsReading)
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		pDatabase->m_IsWriting = true;
	}

	AssetDatabase::WriteLock::~WriteLock()
	{
		--m_LockCounter;
		if (m_LockCounter > 0) return;
		m_pDatabase->m_IsWriting = false;
	}
}
