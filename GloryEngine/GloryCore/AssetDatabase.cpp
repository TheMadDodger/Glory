#include <fstream>
#include <filesystem>
#include "AssetDatabase.h"
#include "AssetManager.h"
#include "LayerManager.h"
#include "Serializer.h"

namespace Glory
{
	ThreadedUMap<UUID, AssetLocation> AssetDatabase::m_AssetLocations;
	ThreadedUMap<std::string, UUID> AssetDatabase::m_PathToUUID;
	ThreadedUMap<UUID, ResourceMeta> AssetDatabase::m_Metas;
	ThreadedUMap<size_t, std::vector<UUID>> AssetDatabase::m_AssetsByType;
	AssetCallbacks AssetDatabase::m_Callbacks;
	ThreadedVector<UUID> AssetDatabase::m_UnsavedAssets;

	bool AssetDatabase::GetAssetLocation(UUID uuid, AssetLocation& location)
	{
		if (!m_AssetLocations.Contains(uuid))
		{
			//throw new std::exception("Asset not found!");
			return false;
		}

		location = m_AssetLocations[uuid];
		return true;
	}

	bool AssetDatabase::GetResourceMeta(UUID uuid, ResourceMeta& meta)
	{
		if (!m_Metas.Contains(uuid))
		{
			//throw new std::exception("Asset not found!");
			return false;
		}

		meta = m_Metas[uuid];
		return true;
	}

	UUID AssetDatabase::GetAssetUUID(const std::string& path)
	{
		if (!m_PathToUUID.Contains(path)) return 0;
		return m_PathToUUID[path];
	}

	bool AssetDatabase::AssetExists(UUID uuid)
	{
		return m_AssetLocations.Contains(uuid);
	}

	bool AssetDatabase::AssetExists(const std::string& path)
	{
		return m_PathToUUID.Contains(path);
	}

	void AssetDatabase::InsertAsset(const std::string& path, const ResourceMeta& meta)
	{
		uint64_t uuid = meta.ID();
		m_Metas.Set(uuid, meta);
		m_AssetLocations.Set(uuid, AssetLocation(path, m_Metas[m_Metas.Size() - 1]));
		m_PathToUUID.Set(path, uuid);
		m_AssetsByType.Do(meta.Hash(), [&](std::vector<UUID>* assets) { assets->push_back(uuid); });
		m_Callbacks.EnqueueCallback(CallbackType::CT_AssetRegistered, uuid, nullptr);
	}

	void AssetDatabase::UpdateAssetPath(UUID uuid, const std::string& newPath)
	{
		UpdateAssetPath(uuid, newPath, newPath + ".gmeta");
	}

	void AssetDatabase::UpdateAssetPath(UUID uuid, const std::string& newPath, const std::string& newMetaPath)
	{
		if (!m_AssetLocations.Contains(uuid)) return;

		AssetLocation location = m_AssetLocations[uuid];
		location.m_Path = newPath;
		m_PathToUUID.Erase(location.m_Path);
		m_PathToUUID.Set(location.m_Path, uuid);
		m_AssetLocations.Set(uuid, location);

		m_Metas.Do(uuid, [&](ResourceMeta* pMeta) { pMeta->m_Path = newMetaPath; });

		Resource* pResource = AssetManager::FindResource(uuid);

		std::filesystem::path path = newPath;
		if (pResource) pResource->SetName(path.filename().replace_extension().string());
		//ResourceMeta meta = m_Metas[uuid];
		//meta.m_Path = newMetaPath;
		//m_Metas.Set(uuid, meta);
	}

	void AssetDatabase::UpdateAssetPaths(const std::string& oldPath, const std::string& newPath)
	{
		UUID id = AssetDatabase::GetAssetUUID(oldPath);
		if (id != 0)
		{
			UpdateAssetPath(id, newPath);
			return;
		}

		std::filesystem::path absolutePath = Game::GetAssetPath();
		absolutePath = absolutePath.append(oldPath);

		std::vector<std::string> relevantAssetPaths;
		std::vector<UUID> relevantAssets;
		m_PathToUUID.ForEach([&](const std::string& key, const UUID& uuid)
		{
			std::filesystem::path absoluteAssetPath = Game::GetAssetPath();
			absoluteAssetPath = absoluteAssetPath.append(key);
			if (absoluteAssetPath.string().find(absolutePath.string()) == std::string::npos) return;
			relevantAssetPaths.push_back(key);
			relevantAssets.push_back(uuid);
		});

		for (size_t i = 0; i < relevantAssets.size(); i++)
		{
			UUID uuid = relevantAssets[i];
			std::string path = relevantAssetPaths[i];
			size_t index = path.find(oldPath);
			size_t length = oldPath.length();
			path = path.replace(index, length, newPath);
			UpdateAssetPath(uuid, path);
		}
	}

	void AssetDatabase::DeleteAsset(UUID uuid)
	{
		AssetLocation location;
		ResourceMeta meta;
		if (!GetAssetLocation(uuid, location)) return;
		if (!GetResourceMeta(uuid, meta)) return;
		m_Metas.Erase(uuid);
		m_AssetLocations.Erase(uuid);
		m_PathToUUID.Erase(location.m_Path);
		m_AssetsByType.Do(meta.Hash(), [&](std::vector<UUID>* assets)
		{
			auto it = std::find(assets->begin(), assets->end(), uuid);
			if (it == assets->end()) return;
			assets->erase(it);
		});
		m_Callbacks.EnqueueCallback(CallbackType::CT_AssetDeleted, uuid, nullptr);
	}

	void AssetDatabase::DeleteAssets(const std::string& path)
	{
		std::filesystem::path absolutePath = Game::GetAssetPath();
		absolutePath = absolutePath.append(path);

		std::vector<std::string> relevantAssetPaths;
		std::vector<UUID> relevantAssets;
		m_PathToUUID.ForEach([&](const std::string& key, const UUID& uuid)
		{
			std::filesystem::path absoluteAssetPath = Game::GetAssetPath();
			absoluteAssetPath = absoluteAssetPath.append(key);
			if (absoluteAssetPath.string().find(absolutePath.string()) == std::string::npos) return;
			relevantAssetPaths.push_back(key);
			relevantAssets.push_back(uuid);
		});

		for (size_t i = 0; i < relevantAssets.size(); i++)
		{
			UUID uuid = relevantAssets[i];
			DeleteAsset(uuid);
		}
	}

	void AssetDatabase::IncrementAssetVersion(UUID uuid)
	{
		if (!m_Metas.Contains(uuid)) return;
		m_Metas.Do(uuid, [&](ResourceMeta* pMeta)
		{
			++pMeta->m_SerializedVersion;
			LoaderModule* pLoader = Game::GetGame().GetEngine()->GetLoaderModule(pMeta->Hash());
			if (!pLoader) return;
			pMeta->Write(pLoader);
		});
	}

	void AssetDatabase::Save()
	{
		SaveDirtyAssets();

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

			std::filesystem::path metaFilePath(path);
			std::filesystem::path metaExtension = std::filesystem::path(".gmeta");
			metaFilePath += std::filesystem::path(".gmeta");
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

	void AssetDatabase::CreateAsset(Resource* pResource, const std::string& path)
	{
		std::filesystem::path filePath = path;
		std::filesystem::path extension = filePath.extension();
		std::filesystem::path fileName = filePath.filename().replace_extension("");
		LoaderModule* pModule = Game::GetGame().GetEngine()->GetLoaderModule(extension.string());
		if (!pModule)
		{
			// Shouldnt happen but uuuuuuuuuh
			Debug::LogError("Failed to save asset, asset type not supported!");
			return;
		}
		pModule->Save(path, pResource);
		ImportAsset(path, pResource);
	}

	void AssetDatabase::ImportAsset(const std::string& path, Resource* pLoadedResource)
	{
		std::filesystem::path filePath = path;
		std::filesystem::path extension = filePath.extension();
		std::filesystem::path fileName = filePath.filename().replace_extension("");

		LoaderModule* pModule = Game::GetGame().GetEngine()->GetLoaderModule(extension.string());
		if (!pModule)
		{
			// Not supperted!
			Debug::LogError("Failed to import file, asset type not supported!");
			return;
		}

		if (!pLoadedResource)
		{
			std::any importSettings = pModule->CreateDefaultImportSettings(extension.string());
			pLoadedResource = pModule->LoadUsingAny(path, importSettings);
		}

		std::filesystem::path metaExtension = std::filesystem::path(".gmeta");
		std::filesystem::path metaFilePath = path + metaExtension.string();
		// Generate a meta file
		const ResourceType* pType = ResourceType::GetResourceType(extension.string());

		if (!pType)
		{
			// Not supperted!
			Debug::LogError("Failed to import file, could not determine ResourceType!");
			return;
		}

		const std::string assetPath = Game::GetAssetPath();
		metaFilePath = metaFilePath.lexically_relative(assetPath);
		ResourceMeta meta(metaFilePath.string(), extension.string(), UUID(), pType->Hash());
		meta.Write(pModule);
		meta.Read();
		pLoadedResource->m_ID = meta.ID();
		pLoadedResource->m_Name = fileName.string();
		std::filesystem::path relativePath = filePath.lexically_relative(Game::GetGame().GetAssetPath());
		AssetManager::m_pLoadedAssets.Set(pLoadedResource->m_ID, pLoadedResource);
		InsertAsset(relativePath.string(), meta);
	}

	void AssetDatabase::ImportNewScene(const std::string& path, GScene* pScene)
	{
		std::filesystem::path filePath = path;
		std::filesystem::path extension = filePath.extension();
		std::filesystem::path fileName = filePath.filename().replace_extension("");
		std::filesystem::path metaExtension = std::filesystem::path(".gmeta");
		std::filesystem::path metaFilePath = path + metaExtension.string();
		// Generate a meta file
		const ResourceType* pType = ResourceType::GetResourceType<GScene>();

		const std::string assetPath = Game::GetAssetPath();
		metaFilePath = metaFilePath.lexically_relative(assetPath);
		ResourceMeta meta(metaFilePath.string(), extension.string(), UUID(), pType->Hash());
		meta.Write(nullptr);
		meta.Read();
		pScene->m_ID = meta.ID();
		pScene->m_Name = fileName.string();
		std::filesystem::path relativePath = filePath.lexically_relative(Game::GetGame().GetAssetPath());
		AssetManager::m_pLoadedAssets.Set(pScene->m_ID, pScene);
		InsertAsset(relativePath.string(), meta);
	}

	void AssetDatabase::SaveAsset(Resource* pResource, bool markUndirty)
	{
		ResourceMeta meta;
		if (!pResource || !GetResourceMeta(pResource->GetUUID(), meta)) return;
		AssetLocation location;
		if (!GetAssetLocation(pResource->GetUUID(), location)) return;
		LoaderModule* pModule = Game::GetGame().GetEngine()->GetLoaderModule(meta.m_TypeHash);
		std::filesystem::path path = Game::GetAssetPath();
		path.append(location.m_Path);
		pModule->Save(path.string(), pResource);
		IncrementAssetVersion(pResource->GetUUID());
		if (markUndirty)
			m_UnsavedAssets.Erase(pResource->GetUUID());
	}

	void AssetDatabase::ForEachAssetLocation(std::function<void(UUID, const AssetLocation&)> callback)
	{
		m_AssetLocations.ForEach(callback);
	}

	void AssetDatabase::RemoveAsset(UUID uuid)
	{
		if (!m_AssetLocations.Contains(uuid))
			return;

		AssetLocation location = m_AssetLocations[uuid];
		std::string path = location.m_Path;
		m_AssetLocations.Erase(uuid);
		m_Metas.Erase(uuid);
		m_PathToUUID.Erase(path);
	}

	void AssetDatabase::GetAllAssetsOfType(size_t typeHash, std::vector<UUID>& out)
	{
		if (!m_AssetsByType.Contains(typeHash)) return;
		size_t size = out.size();
		out.resize(size + m_AssetsByType[typeHash].size());
		size_t copySize = sizeof(UUID) * m_AssetsByType[typeHash].size();
		memcpy(&out[size], &m_AssetsByType[typeHash][0], copySize);
	}

	std::string AssetDatabase::GetAssetName(UUID uuid)
	{
		// Will need to be different for build and editor since assets are grouped together in a single file when built
		AssetLocation location;
		if (!GetAssetLocation(uuid, location)) return "";
		std::filesystem::path path(location.m_Path);
		return path.filename().replace_extension("").string();
	}

	void AssetDatabase::SetDirty(Object* pResource)
	{
		SetDirty(pResource->GetUUID());
	}

	void AssetDatabase::SetDirty(UUID uuid)
	{
		if (m_UnsavedAssets.Contains(uuid)) return;
		m_UnsavedAssets.push_back(uuid);
	}

	void AssetDatabase::SaveDirtyAssets()
	{
		m_UnsavedAssets.ForEachClear([&](const UUID& uuid)
		{
			Resource* pResource = AssetManager::FindResource(uuid);
			if (!pResource) return;
			SaveAsset(pResource, false);
		});
	}

	void AssetDatabase::Initialize()
	{
		m_Callbacks.Initialize();
	}

	void AssetDatabase::Destroy()
	{
		Save();
		m_Callbacks.Cleanup();
		Clear();
	}

	void AssetDatabase::Clear()
	{
		m_AssetLocations.Clear();
		m_PathToUUID.Clear();
		m_Metas.Clear();
		m_AssetsByType.Clear();
	}

	void AssetDatabase::ExportEditor(YAML::Emitter& out)
	{
		out << YAML::BeginSeq;

		m_AssetLocations.ForEach([&](const UUID& uuid, const AssetLocation& location)
		{
			if (!m_Metas.Contains(uuid))
				return;

			ResourceMeta meta = m_Metas[uuid];

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
		});

		out << YAML::EndSeq;
	}

	void AssetDatabase::ExportBuild(YAML::Emitter& out)
	{
		
	}

	AssetDatabase::AssetDatabase() {}

	AssetDatabase::~AssetDatabase() {}
}
