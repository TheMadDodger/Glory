#include "EditorAssetDatabase.h"
#include "AssetManager.h"
#include <Engine.h>
#include <ProjectSpace.h>

namespace Glory::Editor
{
	YAML::Node EditorAssetDatabase::m_DatabaseNode;
	ThreadedVector<UUID> EditorAssetDatabase::m_UnsavedAssets;
	ThreadedUMap<UUID, long> EditorAssetDatabase::m_LastSavedRecords;
	bool EditorAssetDatabase::m_IsDirty;

	void EditorAssetDatabase::Load(YAML::Node& projectNode)
	{
		YAML::Node assetsNode = projectNode["Assets"];
		if (!projectNode["Assets"].IsDefined() || !projectNode["Assets"].IsMap())
		{
			projectNode["Assets"] = YAML::Node(YAML::NodeType::Map);
		}

		m_DatabaseNode = projectNode["Assets"];

		AssetDatabase::Load(m_DatabaseNode);

		for (YAML::const_iterator itor = m_DatabaseNode.begin(); itor != m_DatabaseNode.end(); ++itor)
		{
			UUID key = itor->first.as<uint64_t>();
			AssetDatabase::EnqueueCallback(CallbackType::CT_AssetRegistered, key, nullptr);
		}
	}

	void EditorAssetDatabase::Reload()
	{
		AssetDatabase::Load(m_DatabaseNode);
	}

	void EditorAssetDatabase::InsertAsset(const std::string& path, const ResourceMeta& meta, bool setDirty)
	{
		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');
		uint64_t uuid = meta.ID();
		const std::string key = std::to_string(uuid);
		YAML::Node assetNode = m_DatabaseNode[key];
		/* We are not updating the asset but adding it */
		/* So if the asset already exists we ignore it */
		if (assetNode.IsDefined() && assetNode.IsMap()) return;

		YAML::Node insertedAsset{ YAML::NodeType::Map };
		insertedAsset["Location"] = AssetLocation(path);
		insertedAsset["Metadata"] = meta;

		m_DatabaseNode[key] = insertedAsset;

		AssetDatabase::EnqueueCallback(CallbackType::CT_AssetRegistered, uuid, nullptr);
		SetDirty();
	}

	void EditorAssetDatabase::UpdateAssetPath(UUID uuid, const std::string& newPath)
	{
		YAML::Node assetNode = m_DatabaseNode[std::to_string(uuid)];

		if (!assetNode.IsDefined() || !assetNode.IsMap()) return;

		std::string fixedNewPath = newPath;
		std::replace(fixedNewPath.begin(), fixedNewPath.end(), '/', '\\');
		AssetLocation location = assetNode["Location"].as<AssetLocation>();
		location.Path = fixedNewPath;
		assetNode["Location"] = location;

		Resource* pResource = AssetManager::FindResource(uuid);

		std::filesystem::path path = fixedNewPath;
		if (pResource) pResource->SetName(path.filename().replace_extension().string());
		SetDirty();
	}

	void EditorAssetDatabase::UpdateAsset(UUID uuid, long lastSaved)
	{
		m_LastSavedRecords.Set(uuid, lastSaved);
		AssetDatabase::EnqueueCallback(CallbackType::CT_AssetUpdated, uuid, nullptr);
		SetDirty();
	}

	long EditorAssetDatabase::GetLastSavedRecord(UUID uuid)
	{
		return m_LastSavedRecords[uuid];
	}

	void EditorAssetDatabase::UpdateAssetPaths(const std::string& oldPath, const std::string& newPath)
	{
		std::string fixedNewPath = newPath;
		std::replace(fixedNewPath.begin(), fixedNewPath.end(), '/', '\\');
		std::string fixedOldPath = oldPath;
		std::replace(fixedOldPath.begin(), fixedOldPath.end(), '/', '\\');

		std::filesystem::path absolutePath = Game::GetAssetPath();
		absolutePath = absolutePath.append(fixedOldPath);

		/* Find all assets inside this folder and update their folder */
		for (YAML::const_iterator itor = m_DatabaseNode.begin(); itor != m_DatabaseNode.end(); ++itor)
		{
			UUID uuid = itor->first.as<uint64_t>();
			const std::string key = std::to_string(uuid);
			YAML::Node assetNode = m_DatabaseNode[key];
			const AssetLocation location = assetNode["Location"].as<AssetLocation>();
			std::filesystem::path absoluteAssetPath = Game::GetAssetPath();
			absoluteAssetPath = absoluteAssetPath.append(location.Path);
			if (absoluteAssetPath.string().find(absolutePath.string()) == std::string::npos) continue;

			std::string path = location.Path;
			size_t index = path.find(fixedOldPath);
			size_t length = fixedOldPath.length();
			path = path.replace(index, length, fixedNewPath);
			UpdateAssetPath(uuid, path);
		}
	}

	void EditorAssetDatabase::DeleteAsset(UUID uuid)
	{
		const std::string key = std::to_string(uuid);
		YAML::Node assetNode = m_DatabaseNode[key];
		if (!assetNode.IsDefined() || !assetNode.IsMap()) return;

		m_DatabaseNode.remove(key);
		AssetDatabase::EnqueueCallback(CallbackType::CT_AssetDeleted, uuid, nullptr);
		SetDirty();
	}

	void EditorAssetDatabase::DeleteAsset(const std::string& path)
	{
		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');

		std::filesystem::path absolutePath = Game::GetAssetPath();
		absolutePath = absolutePath.append(fixedPath);

		std::vector<UUID> relevantAssets;
		for (YAML::const_iterator itor = m_DatabaseNode.begin(); itor != m_DatabaseNode.end(); ++itor)
		{
			UUID uuid = itor->first.as<uint64_t>();
			const std::string key = std::to_string(uuid);
			YAML::Node assetNode = m_DatabaseNode[key];

			const AssetLocation location = assetNode["Location"].as<AssetLocation>();
			std::filesystem::path absoluteAssetPath = Game::GetAssetPath();
			absoluteAssetPath = absoluteAssetPath.append(location.Path);
			if (absoluteAssetPath.string().find(absolutePath.string()) == std::string::npos) continue;
			DeleteAsset(uuid);
			return;
		}
	}

	void EditorAssetDatabase::DeleteAssets(const std::string& path)
	{
		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');

		std::filesystem::path absolutePath = Game::GetAssetPath();
		absolutePath = absolutePath.append(fixedPath);

		std::vector<UUID> relevantAssets;
		for (YAML::const_iterator itor = m_DatabaseNode.begin(); itor != m_DatabaseNode.end(); ++itor)
		{
			UUID uuid = itor->first.as<uint64_t>();
			const std::string key = std::to_string(uuid);
			YAML::Node assetNode = m_DatabaseNode[key];

			const AssetLocation location = assetNode["Location"].as<AssetLocation>();
			std::filesystem::path absoluteAssetPath = Game::GetAssetPath();
			absoluteAssetPath = absoluteAssetPath.append(location.Path);
			if (absoluteAssetPath.string().find(absolutePath.string()) == std::string::npos) continue;
			relevantAssets.push_back(uuid);
		}

		for (size_t i = 0; i < relevantAssets.size(); i++)
		{
			UUID uuid = relevantAssets[i];
			DeleteAsset(uuid);
		}
	}

	void EditorAssetDatabase::IncrementAssetVersion(UUID uuid)
	{
		YAML::Node assetNode = m_DatabaseNode[std::to_string(uuid)];
		if (!assetNode.IsDefined() || !assetNode.IsMap()) return;
		ResourceMeta meta = assetNode["Metadata"].as<ResourceMeta>();
		meta.IncrementSerializedVersion();
		assetNode["Metadata"] = meta;
		SetDirty();
	}

	void EditorAssetDatabase::CreateAsset(Resource* pResource, const std::string& path)
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

	void EditorAssetDatabase::ImportAsset(const std::string& path, Resource* pLoadedResource)
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
		ResourceMeta meta(extension.string(), UUID(), pType->Hash());

		AssetDatabase::SetIDAndName(pLoadedResource, meta.ID(), fileName.string());
		std::filesystem::path relativePath = filePath.lexically_relative(Game::GetGame().GetAssetPath());
		AssetManager::AddLoadedResource(pLoadedResource);
		InsertAsset(relativePath.string(), meta);
	}

	void EditorAssetDatabase::ImportNewScene(const std::string& path, GScene* pScene)
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
		ResourceMeta meta(extension.string(), pScene->GetUUID(), pType->Hash());

		AssetDatabase::SetIDAndName(pScene, meta.ID(), fileName.string());
		std::filesystem::path relativePath = filePath.lexically_relative(Game::GetGame().GetAssetPath());
		AssetManager::AddLoadedResource(pScene);
		InsertAsset(relativePath.string(), meta);
	}

	void EditorAssetDatabase::SaveAsset(Resource* pResource, bool markUndirty)
	{
		if (!pResource) return;
		const UUID uuid = pResource->GetUUID();
		const std::string key = std::to_string(uuid);
		YAML::Node assetNode = m_DatabaseNode[key];

		if (!assetNode.IsDefined() || !assetNode.IsMap()) return;

		AssetLocation location = assetNode["Location"].as<AssetLocation>();
		ResourceMeta meta = assetNode["Metadata"].as<ResourceMeta>();

		LoaderModule* pModule = Game::GetGame().GetEngine()->GetLoaderModule(meta.Hash());
		std::filesystem::path path = Game::GetAssetPath();
		path.append(location.Path);
		pModule->Save(path.string(), pResource);
		IncrementAssetVersion(pResource->GetUUID());

		if (markUndirty)
			m_UnsavedAssets.Erase(pResource->GetUUID());
	}

	void EditorAssetDatabase::RemoveAsset(UUID uuid)
	{
		const std::string key = std::to_string(uuid);
		YAML::Node assetNode = m_DatabaseNode[key];
		if (!assetNode.IsDefined() || !assetNode.IsMap()) return;
		m_DatabaseNode.remove(key);
		SetDirty();
	}

	void EditorAssetDatabase::SetAssetDirty(Object* pResource)
	{
		SetAssetDirty(pResource->GetUUID());
	}

	void EditorAssetDatabase::SetAssetDirty(UUID uuid)
	{
		if (m_UnsavedAssets.Contains(uuid)) return;
		m_UnsavedAssets.push_back(uuid);
		SetDirty();
	}

	void EditorAssetDatabase::SaveDirtyAssets()
	{
		m_UnsavedAssets.ForEachClear([&](const UUID& uuid)
		{
			Resource* pResource = AssetManager::FindResource(uuid);
			if (!pResource) return;
			SaveAsset(pResource, false);
		});
	}

	void EditorAssetDatabase::SetDirty(bool dirty)
	{
		m_IsDirty = dirty;
		ProjectSpace::SetAssetDirty("AssetDatabase", dirty);
	}

	bool EditorAssetDatabase::IsDirty()
	{
		return m_IsDirty || m_UnsavedAssets.Size();
	}

	bool EditorAssetDatabase::GetAssetLocation(UUID uuid, AssetLocation& location)
	{
		const std::string key = std::to_string(uuid);
		YAML::Node assetNode = m_DatabaseNode[key];
		if (!assetNode.IsDefined() || !assetNode.IsMap()) return false;
		location = assetNode["Location"].as<AssetLocation>();
		return true;
	}

	bool EditorAssetDatabase::GetAssetMetadata(UUID uuid, ResourceMeta& meta)
	{
		const std::string key = std::to_string(uuid);
		YAML::Node assetNode = m_DatabaseNode[key];
		if (!assetNode.IsDefined() || !assetNode.IsMap()) return false;
		meta = assetNode["Metadata"].as<ResourceMeta>();
		return true;
	}

	UUID EditorAssetDatabase::FindAssetUUID(const std::string& path)
	{
		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');

		std::filesystem::path absolutePath = Game::GetAssetPath();
		absolutePath = absolutePath.append(fixedPath);

		for (YAML::const_iterator itor = m_DatabaseNode.begin(); itor != m_DatabaseNode.end(); ++itor)
		{
			UUID uuid = itor->first.as<uint64_t>();
			const std::string key = std::to_string(uuid);
			YAML::Node assetNode = m_DatabaseNode[key];

			const AssetLocation location = assetNode["Location"].as<AssetLocation>();
			std::filesystem::path absoluteAssetPath = Game::GetAssetPath();
			absoluteAssetPath = absoluteAssetPath.append(location.Path);
			if (absoluteAssetPath.string().find(absolutePath.string()) == std::string::npos) continue;
			return uuid;
		}

		return 0;
	}

	bool EditorAssetDatabase::AssetExists(UUID uuid)
	{
		const std::string key = std::to_string(uuid);
		YAML::Node assetNode = m_DatabaseNode[key];
		return !assetNode.IsDefined() && !assetNode.IsMap();
	}

	std::string EditorAssetDatabase::GetAssetName(UUID uuid)
	{
		const std::string key = std::to_string(uuid);
		YAML::Node assetNode = m_DatabaseNode[key];
		if (!assetNode.IsDefined() || !assetNode.IsMap()) return "";
		AssetLocation location = assetNode["Location"].as<AssetLocation>();
		return std::filesystem::path(location.Path).filename().replace_extension().string();
	}

	void EditorAssetDatabase::GetAllAssetsOfType(size_t typeHash, std::vector<UUID>& result)
	{
		for (YAML::const_iterator itor = m_DatabaseNode.begin(); itor != m_DatabaseNode.end(); ++itor)
		{
			UUID uuid = itor->first.as<uint64_t>();
			const std::string key = std::to_string(uuid);
			YAML::Node assetNode = m_DatabaseNode[key];

			const ResourceMeta meta = assetNode["Metadata"].as<ResourceMeta>();
			if (meta.Hash() != typeHash) continue;
			result.push_back(uuid);
		}
	}
}
