#include <fstream>
#include <filesystem>
#include "AssetDatabase.h"
#include "AssetManager.h"
#include "LayerManager.h"
#include "Serializer.h"
#include "GloryContext.h"
#include "Engine.h"

namespace Glory
{
	bool AssetDatabase::GetAssetLocation(UUID uuid, AssetLocation& location)
	{
		ReadLock readLock;
		if (ASSET_DATABASE->m_AssetLocations.find(uuid) == ASSET_DATABASE->m_AssetLocations.end())
		{
			return false;
		}

		location = ASSET_DATABASE->m_AssetLocations[uuid];
		return true;
	}

	bool AssetDatabase::GetResourceMeta(UUID uuid, ResourceMeta& meta)
	{
		ReadLock readLock;
		if (ASSET_DATABASE->m_Metas.find(uuid) == ASSET_DATABASE->m_Metas.end())
		{
			return false;
		}

		meta = ASSET_DATABASE->m_Metas.at(uuid);
		return true;
	}

	UUID AssetDatabase::GetAssetUUID(const std::string& path)
	{
		ReadLock readLock;
		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');
		UUID uuid = ASSET_DATABASE->m_PathToUUID[fixedPath];
		return uuid;
	}

	bool AssetDatabase::AssetExists(UUID uuid)
	{
		ReadLock readLock;
		return ASSET_DATABASE->m_AssetLocations.find(uuid) != ASSET_DATABASE->m_AssetLocations.end();
	}

	bool AssetDatabase::AssetExists(const std::string& path)
	{
		ReadLock readLock;

		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');
		return ASSET_DATABASE->m_PathToUUID.find(fixedPath) != ASSET_DATABASE->m_PathToUUID.end();
	}

	void AssetDatabase::EnqueueCallback(const CallbackType& type, UUID uuid, Resource* pResource)
	{
		ASSET_DATABASE->m_Callbacks.EnqueueCallback(type, uuid, pResource);
	}

	void AssetDatabase::SetAsset(AssetLocation& assetLocation, const ResourceMeta& meta)
	{
		std::replace(assetLocation.Path.begin(), assetLocation.Path.end(), '/', '\\');

		const uint64_t uuid = meta.ID();
		ASSET_DATABASE->m_Metas[uuid] = meta;
		ASSET_DATABASE->m_AssetLocations[uuid] = assetLocation;
		if (assetLocation.SubresourcePath.empty()) ASSET_DATABASE->m_PathToUUID[assetLocation.Path] = uuid;
		ASSET_DATABASE->m_AssetsByType[meta.m_TypeHash].push_back(uuid);
	}

	void AssetDatabase::Remove(UUID uuid)
	{
		const ResourceMeta& meta = ASSET_DATABASE->m_Metas[uuid];
		const AssetLocation& location = ASSET_DATABASE->m_AssetLocations[uuid];

		if (location.SubresourcePath.empty()) ASSET_DATABASE->m_PathToUUID.erase(location.Path);
		std::remove(ASSET_DATABASE->m_AssetsByType[meta.m_TypeHash].begin(), ASSET_DATABASE->m_AssetsByType[meta.m_TypeHash].end(), uuid);
		ASSET_DATABASE->m_AssetLocations.erase(uuid);
		ASSET_DATABASE->m_Metas.erase(uuid);
	}

	void AssetDatabase::Load()
	{
		while (ASSET_DATABASE->m_IsReading)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		ASSET_DATABASE->m_Initialized = false;
		

		ASSET_DATABASE->m_Initialized = true;
	}

	void AssetDatabase::GetAllAssetsOfType(uint32_t typeHash, std::vector<UUID>& out)
	{
		ReadLock readLock;

		if (ASSET_DATABASE->m_AssetsByType.find(typeHash) == ASSET_DATABASE->m_AssetsByType.end()) return;
		size_t size = out.size();
		out.resize(size + ASSET_DATABASE->m_AssetsByType[typeHash].size());
		size_t copySize = sizeof(UUID) * ASSET_DATABASE->m_AssetsByType[typeHash].size();
		memcpy(&out[size], &ASSET_DATABASE->m_AssetsByType[typeHash][0], copySize);
	}

	void AssetDatabase::GetAllAssetsOfType(uint32_t typeHash, std::vector<std::string>& out)
	{
		ReadLock readLock;

		if (ASSET_DATABASE->m_AssetsByType.find(typeHash) == ASSET_DATABASE->m_AssetsByType.end()) return;
		size_t size = out.size();
		out.resize(size + ASSET_DATABASE->m_AssetsByType.at(typeHash).size());
		for (size_t i = 0; i < ASSET_DATABASE->m_AssetsByType.at(typeHash).size(); i++)
		{
			UUID uuid = ASSET_DATABASE->m_AssetsByType.at(typeHash)[i];
			AssetLocation location;
			GetAssetLocation(uuid, location);
			out.push_back(location.Path);
		}
	}

	std::string AssetDatabase::GetAssetName(UUID uuid)
	{
		ReadLock readLock;

		// Will need to be different for build and editor since assets are grouped together in a single file when built
		AssetLocation location;
		if (!GetAssetLocation(uuid, location)) return "";
		std::filesystem::path path(location.Path);
		return path.filename().replace_extension("").string();
	}

	void AssetDatabase::Initialize()
	{
		ASSET_DATABASE->m_Callbacks.Initialize();
	}

	void AssetDatabase::Destroy()
	{
		ASSET_DATABASE->m_Callbacks.Cleanup();
		Clear();
	}

	void AssetDatabase::Clear()
	{
		while (ASSET_DATABASE->m_IsReading)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		ASSET_DATABASE->m_Initialized = false;
		ASSET_DATABASE->m_AssetLocations.clear();
		ASSET_DATABASE->m_PathToUUID.clear();
		ASSET_DATABASE->m_Metas.clear();
		ASSET_DATABASE->m_AssetsByType.clear();
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
	//	ASSET_DATABASE->m_AssetLocations.ForEach([&](const UUID& uuid, const AssetLocation& location)
	//	{
	//		if (!ASSET_DATABASE->m_Metas.Contains(uuid))
	//			return;
	//
	//		ResourceMeta meta = ASSET_DATABASE->m_Metas[uuid];
	//
	//		size_t versionHash = std::hash<ResourceMeta>()(meta);
	//		long lastSaved = ASSET_DATABASE->m_LastSavedRecords[uuid];
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
		: m_Initialized(false), m_IsReading(false) {}

	AssetDatabase::~AssetDatabase() {}

	size_t AssetDatabase::ReadLock::m_LockCounter = 0;
	size_t AssetDatabase::WriteLock::m_LockCounter = 0;

	AssetDatabase::ReadLock::ReadLock()
	{
		++m_LockCounter;
		while (!ASSET_DATABASE->m_Initialized)
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		ASSET_DATABASE->m_IsReading = true;
	}

	AssetDatabase::ReadLock::~ReadLock()
	{
		--m_LockCounter;
		if (m_LockCounter > 0) return;
		ASSET_DATABASE->m_IsReading = false;
	}

	AssetDatabase::WriteLock::WriteLock()
	{
		++m_LockCounter;
		while (ASSET_DATABASE->m_IsReading)
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		ASSET_DATABASE->m_Initialized = false;
	}

	AssetDatabase::WriteLock::~WriteLock()
	{
		--m_LockCounter;
		if (m_LockCounter > 0) return;
		ASSET_DATABASE->m_Initialized = true;
	}
}
