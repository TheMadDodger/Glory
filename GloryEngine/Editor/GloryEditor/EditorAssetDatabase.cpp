#include "EditorAssetDatabase.h"
#include "AssetManager.h"
#include "EditorAssetCallbacks.h"
#include <Engine.h>
#include <ProjectSpace.h>
#include <FileBrowser.h>
#include <EditorSceneManager.h>

namespace Glory::Editor
{
	YAML::Node EditorAssetDatabase::m_DatabaseNode;
	ThreadedVector<UUID> EditorAssetDatabase::m_UnsavedAssets;
	ThreadedVector<EditorAssetDatabase::ImportedResource> EditorAssetDatabase::m_ImportedResources;
	ThreadedUMap<std::string, UUID> EditorAssetDatabase::m_PathToUUIDCache;
	ThreadedUMap<UUID, long> EditorAssetDatabase::m_LastSavedRecords;
	std::function<void(Resource*)> EditorAssetDatabase::m_AsyncImportCallback;
	bool EditorAssetDatabase::m_IsDirty;

	Jobs::JobPool<bool, std::filesystem::path>* EditorAssetDatabase::m_pImportPool = nullptr;

	void EditorAssetDatabase::Load(YAML::Node& projectNode)
	{
		YAML::Node assetsNode = projectNode["Assets"];
		if (!projectNode["Assets"].IsDefined() || !projectNode["Assets"].IsMap())
		{
			projectNode["Assets"] = YAML::Node(YAML::NodeType::Map);
		}

		m_DatabaseNode = projectNode["Assets"];

#if OLD_TO_NEW_HASH
		for (YAML::const_iterator itor = m_DatabaseNode.begin(); itor != m_DatabaseNode.end(); ++itor)
		{
			const UUID uuid = itor->first.as<uint64_t>();
			EditorAssetCallbacks::EnqueueCallback(AssetCallbackType::CT_AssetRegistered, uuid, nullptr);

			const std::string key = std::to_string(uuid);
			YAML::Node assetNode = m_DatabaseNode[key];
			YAML::Node metaData = assetNode["Metadata"];
			uint32_t hash = metaData["Hash"].as<size_t>();
			size_t newHash = ResourceType::OldToNewHash(hash);
			if (hash != newHash)
				metaData["Hash"] = newHash;
		}
#endif

		AssetDatabase::Load(m_DatabaseNode);

		m_PathToUUIDCache.Clear();
		for (YAML::const_iterator itor = m_DatabaseNode.begin(); itor != m_DatabaseNode.end(); ++itor)
		{
			const UUID uuid = itor->first.as<uint64_t>();
			EditorAssetCallbacks::EnqueueCallback(AssetCallbackType::CT_AssetRegistered, uuid, nullptr);

			const std::string key = std::to_string(uuid);
			YAML::Node assetNode = m_DatabaseNode[key];
			const AssetLocation location = assetNode["Location"].as<AssetLocation>();
			if (!location.SubresourcePath.empty()) continue;

			std::filesystem::path absolutePath = location.Path;
			if (!absolutePath.is_absolute() && location.Path[0] != '.')
			{
				absolutePath = Game::GetAssetPath();
				absolutePath = absolutePath.append(location.Path);
			}

			m_PathToUUIDCache.Set(absolutePath.string(), uuid);
		}

		Debug::LogInfo("Loaded asset database");

		m_LastSavedRecords.Clear();
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path path = pProject->CachePath();
		path.append("LastSaved.yaml");
		if (!std::filesystem::exists(path)) return;
		YAML::Node savedRecordsNode = YAML::LoadFile(path.string());
		if (!savedRecordsNode.IsDefined() || !savedRecordsNode.IsMap()) return;
		for (YAML::const_iterator itor = savedRecordsNode.begin(); itor != savedRecordsNode.end(); ++itor)
		{
			const UUID uuid = itor->first.as<uint64_t>();
			const long lastSaved = itor->second.as<long>();
			m_LastSavedRecords.Set(uuid, lastSaved);
		}
	}

	void EditorAssetDatabase::Reload()
	{
		AssetDatabase::Clear();
		AssetDatabase::Load(m_DatabaseNode);

		Debug::LogInfo("Reloaded asset database");
	}

	void EditorAssetDatabase::InsertAsset(AssetLocation& location, const ResourceMeta& meta, bool setDirty)
	{
		std::replace(location.Path.begin(), location.Path.end(), '/', '\\');
		uint64_t uuid = meta.ID();
		const std::string key = std::to_string(uuid);
		YAML::Node assetNode = m_DatabaseNode[key];
		/* We are not updating the asset but adding it */
		/* So if the asset already exists we ignore it */
		if (assetNode.IsDefined() && assetNode.IsMap()) return;

		YAML::Node insertedAsset{ YAML::NodeType::Map };
		insertedAsset["Location"] = location;
		insertedAsset["Metadata"] = meta;

		m_DatabaseNode[key] = insertedAsset;

		EditorAssetCallbacks::EnqueueCallback(AssetCallbackType::CT_AssetRegistered, uuid, nullptr);
		SetDirty();
	}

	void EditorAssetDatabase::UpdateAssetPath(UUID uuid, const std::string& newPath)
	{
		YAML::Node assetNode = m_DatabaseNode[std::to_string(uuid)];

		if (!assetNode.IsDefined() || !assetNode.IsMap()) return;

		std::string fixedNewPath = newPath;
		std::replace(fixedNewPath.begin(), fixedNewPath.end(), '/', '\\');
		AssetLocation location = assetNode["Location"].as<AssetLocation>();
		const std::string oldPath = location.Path;
		location.Path = fixedNewPath;
		assetNode["Location"] = location;
		assetNode["Metadata"]["Name"] = std::filesystem::path{ newPath }.filename().replace_extension().string();

		std::filesystem::path oldAbsolutePath = oldPath;
		if (!oldAbsolutePath.is_absolute() && oldPath[0] != '.')
		{
			oldAbsolutePath = Game::GetAssetPath();
			oldAbsolutePath = oldAbsolutePath.append(oldPath);
		}
		std::filesystem::path newAbsolutePath = newPath;
		if (!newAbsolutePath.is_absolute() && newPath[0] != '.')
		{
			newAbsolutePath = Game::GetAssetPath();
			newAbsolutePath = newAbsolutePath.append(newPath);
		}
		m_PathToUUIDCache.Erase(oldAbsolutePath.string());
		m_PathToUUIDCache.Set(newAbsolutePath.string(), uuid);

		std::stringstream stream;
		stream << "Moved asset from " << oldPath << " to " << fixedNewPath;
		Debug::LogInfo(stream.str());

		SetDirty();
	}

	void EditorAssetDatabase::UpdateAsset(UUID uuid, long lastSaved)
	{
		m_LastSavedRecords.Set(uuid, lastSaved);
		EditorAssetCallbacks::EnqueueCallback(AssetCallbackType::CT_AssetUpdated, uuid, nullptr);
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
		EditorAssetCallbacks::EnqueueCallback(AssetCallbackType::CT_AssetDeleted, uuid, nullptr);

		std::stringstream stream;
		stream << "Deleted asset: " << uuid;
		Debug::LogInfo(stream.str());
		SetDirty();
	}

	void EditorAssetDatabase::DeleteAsset(const std::string& path)
	{
		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');

		std::filesystem::path absolutePath = Game::GetAssetPath();
		absolutePath = absolutePath.append(fixedPath);

		/* Find all assets on this path */
		std::vector<UUID> relevantAssets;
		for (YAML::const_iterator itor = m_DatabaseNode.begin(); itor != m_DatabaseNode.end(); ++itor)
		{
			UUID uuid = itor->first.as<uint64_t>();
			const std::string key = std::to_string(uuid);
			YAML::Node assetNode = m_DatabaseNode[key];

			const AssetLocation location = assetNode["Location"].as<AssetLocation>();
			std::filesystem::path absoluteAssetPath = Game::GetAssetPath();
			absoluteAssetPath = absoluteAssetPath.append(location.Path);
			if (absoluteAssetPath != absolutePath.string()) continue;
			relevantAssets.push_back(uuid);
		}

		for (size_t i = 0; i < relevantAssets.size(); i++)
		{
			UUID uuid = relevantAssets[i];
			DeleteAsset(uuid);
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

	void EditorAssetDatabase::ImportAsset(const std::string& path, Resource* pLoadedResource, std::filesystem::path subPath)
	{
		std::filesystem::path filePath = path;
		std::filesystem::path extension = filePath.extension();
		std::filesystem::path fileName = filePath.filename().replace_extension("");

		std::string ext = extension.string();
		std::for_each(ext.begin(), ext.end(), [](char& c) { c = std::tolower(c); });

		if (ResourceType::IsScene(ext))
		{
			ImportScene(path);
			return;
		}

		LoaderModule* pModule = Game::GetGame().GetEngine()->GetLoaderModule(ext);
		if (!pModule)
		{
			// Not supperted!
			Debug::LogError("Failed to import file, asset type not supported!");
			return;
		}

		const ResourceType* pType = nullptr;
		if (!pLoadedResource)
		{
			std::any importSettings = pModule->CreateDefaultImportSettings(ext);
			pLoadedResource = pModule->Load(path);

			if (!pLoadedResource)
			{
				Debug::LogError("Failed to import file, could not load resource file!");
				return;
			}
		}

		if (pLoadedResource)
		{
			/* Try getting the resource type from the loaded resource */
			std::type_index type = typeid(Resource);
			for (size_t i = 0; i < pLoadedResource->TypeCount(); ++i)
			{
				if (!pLoadedResource->GetType(0, type)) continue;
				pType = ResourceType::GetResourceType(type);
				if (pType) break;
			}
		}

		if (!pType)
		{
			/* Try to get the resource type from the extension */
			pType = ResourceType::GetResourceType(ext);

			if (!pType)
			{
				// Not supperted!
				Debug::LogError("Failed to import file, could not determine ResourceType!");
				return;
			}
		}

		// Generate a meta file
		const std::string assetPath = Game::GetAssetPath();

		std::filesystem::path namePath = fileName;
		if (!subPath.empty()) namePath.append(subPath.string());
		ResourceMeta meta(extension.string(), namePath.string(), pLoadedResource->GetUUID(), pType->Hash());

		AssetDatabase::SetIDAndName(pLoadedResource, meta.ID(), meta.Name());
		std::filesystem::path relativePath = filePath.lexically_relative(assetPath);
		AssetManager::AddLoadedResource(pLoadedResource);

		AssetLocation location{ relativePath.empty() ? path : relativePath.string(), subPath.string() };
		InsertAsset(location, meta);

		std::stringstream stream;
		if (!subPath.empty())
			stream << "Imported subasset " << subPath.string() << " at " << path;
		else stream << "Imported asset at " << path;
		Debug::LogInfo(stream.str());

		/* Import sub resources */
		for (size_t i = 0; i < pLoadedResource->SubResourceCount(); i++)
		{
			Resource* pSubResource = pLoadedResource->Subresource(i);
			std::filesystem::path newSubPath = subPath;
			newSubPath.append(pSubResource->Name());
			ImportAsset(path, pSubResource, newSubPath);
		}

		if (!subPath.empty()) return;
		AssetDatabase::Clear();
		AssetDatabase::Load(m_DatabaseNode);
	}

	void EditorAssetDatabase::ImportAssetsAsync(const std::string& path)
	{
		for (auto itor : std::filesystem::directory_iterator(path))
		{
			const std::filesystem::path dir = itor.path();
			const std::string dirString = dir.string();
			if (itor.is_directory())
			{
				ImportAssetsAsync(dirString);
				continue;
			}
			ImportAssetAsync(dirString);
		}
	}

	void EditorAssetDatabase::ImportAssetAsync(const std::string& path)
	{
		std::stringstream stream;
		stream << "Importing " << path << "...";
		Debug::LogInfo(stream.str());
		m_pImportPool->QueueSingleJob(ImportJob, path);
	}

	void EditorAssetDatabase::ImportNewScene(const std::string& path, GScene* pScene)
	{
		std::stringstream stream;
		stream << "Importing new scene at " << path << "...";
		Debug::LogInfo(stream.str());

		std::filesystem::path filePath = path;
		std::filesystem::path extension = filePath.extension();
		std::filesystem::path fileName = filePath.filename().replace_extension("");
		std::filesystem::path metaExtension = std::filesystem::path(".gmeta");

		// Generate a meta
		const ResourceType* pType = ResourceType::GetResourceType<GScene>();

		const std::string assetPath = Game::GetAssetPath();
		ResourceMeta meta(extension.string(), fileName.string(), pScene->GetUUID(), pType->Hash());

		AssetDatabase::SetIDAndName(pScene, meta.ID(), fileName.string());
		std::filesystem::path relativePath = filePath.lexically_relative(Game::GetGame().GetAssetPath());
		AssetManager::AddLoadedResource(pScene);
		AssetLocation location{ relativePath.string() };
		InsertAsset(location, meta);

		stream.clear();
		stream << "Imported new scene: " << pScene->Name();
		Debug::LogInfo(stream.str());
	}

	void EditorAssetDatabase::ImportScene(const std::string& path)
	{
		std::stringstream stream;
		stream << "Importing scene at " << path << "...";
		Debug::LogInfo(stream.str());

		std::filesystem::path filePath = path;
		std::filesystem::path extension = filePath.extension();
		std::filesystem::path fileName = filePath.filename().replace_extension("");
		std::filesystem::path metaExtension = std::filesystem::path(".gmeta");

		// Generate a meta
		const ResourceType* pType = ResourceType::GetResourceType<GScene>();

		const std::string assetPath = Game::GetAssetPath();
		ResourceMeta meta(extension.string(), fileName.string(), UUID(), pType->Hash());

		std::filesystem::path relativePath = filePath.lexically_relative(Game::GetGame().GetAssetPath());
		AssetLocation location{ relativePath.string() };
		InsertAsset(location, meta);

		stream.clear();
		stream << "Imported scene: " << GetAssetName(meta.ID());
		Debug::LogInfo(stream.str());
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

		std::stringstream stream;
		stream << "Saved asset to " << location.Path;
		Debug::LogInfo(stream.str());
	}

	void EditorAssetDatabase::RemoveAsset(UUID uuid)
	{
		const std::string key = std::to_string(uuid);
		YAML::Node assetNode = m_DatabaseNode[key];
		if (!assetNode.IsDefined() || !assetNode.IsMap()) return;
		m_DatabaseNode.remove(key);
		SetDirty();

		std::stringstream stream;
		stream << "Removed asset " << uuid;
		Debug::LogInfo(stream.str());
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

		std::filesystem::path absolutePath = fixedPath;
		if (!absolutePath.is_absolute() && fixedPath[0] != '.')
		{
			absolutePath = Game::GetAssetPath();
			absolutePath = absolutePath.append(fixedPath);
		}

		if (!m_PathToUUIDCache.Contains(absolutePath.string())) return 0;
		return m_PathToUUIDCache[absolutePath.string()];

		//for (YAML::const_iterator itor = m_DatabaseNode.begin(); itor != m_DatabaseNode.end(); ++itor)
		//{
		//	UUID uuid = itor->first.as<uint64_t>();
		//	const std::string key = std::to_string(uuid);
		//	YAML::Node assetNode = m_DatabaseNode[key];
		//
		//	const AssetLocation location = assetNode["Location"].as<AssetLocation>();
		//	std::filesystem::path absoluteAssetPath = location.Path;
		//	if (!absoluteAssetPath.is_absolute() && location.Path[0] != '.')
		//	{
		//		absoluteAssetPath = Game::GetAssetPath();
		//		absoluteAssetPath.append(location.Path);
		//	}
		//	if (absoluteAssetPath.string().find(absolutePath.string()) == std::string::npos) continue;
		//	return uuid;
		//}

		return 0;
	}

	bool EditorAssetDatabase::AssetExists(UUID uuid)
	{
		const std::string key = std::to_string(uuid);
		YAML::Node assetNode = m_DatabaseNode[key];
		return assetNode.IsDefined() && assetNode.IsMap();
	}

	std::string EditorAssetDatabase::GetAssetName(UUID uuid)
	{
		const std::string key = std::to_string(uuid);
		YAML::Node assetNode = m_DatabaseNode[key];
		if (!assetNode.IsDefined() || !assetNode.IsMap()) return "";
		ResourceMeta meta = assetNode["Metadata"].as<ResourceMeta>();
		if (!meta.Name().empty()) return meta.Name();
		AssetLocation location = assetNode["Location"].as<AssetLocation>();
		std::filesystem::path fileName = std::filesystem::path(location.Path).filename().replace_extension();
		if (!location.SubresourcePath.empty()) fileName.append(location.SubresourcePath);
		return fileName.string();
	}

	void EditorAssetDatabase::GetAllAssetsOfType(uint32_t typeHash, std::vector<UUID>& result)
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

	void EditorAssetDatabase::RegisterAsyncImportCallback(std::function<void(Resource*)> func)
	{
		m_AsyncImportCallback = func;
	}

	void EditorAssetDatabase::ImportModuleAssets()
	{
		for (size_t i = 0; i < Game::GetGame().GetEngine()->ModulesCount(); i++)
		{
			Module* pModule = Game::GetGame().GetEngine()->GetModule(i);
			const ModuleMetaData& metaData = pModule->GetMetaData();
			std::filesystem::path assetsPath = metaData.Path().parent_path();
			if (!std::filesystem::exists(assetsPath)) continue;
			assetsPath.append("Assets");
			if (!std::filesystem::exists(assetsPath)) continue;
			ImportModuleAssets(assetsPath);
		}
	}

	void EditorAssetDatabase::Initialize()
	{
		m_pImportPool = Jobs::JobManager::Run<bool, std::filesystem::path>();
		EditorAssetCallbacks::Initialize();

		YAML::Node m_LastSavedNode;
		Debug::LogInfo("Initialized EditorAssetDatabase");
	}

	void EditorAssetDatabase::Cleanup()
	{
		Debug::LogInfo("Cleanup EditorAssetDatabase");

		m_pImportPool = nullptr;
		m_AsyncImportCallback = NULL;

		EditorAssetCallbacks::Cleanup();

		YAML::Emitter out;
		out << YAML::BeginMap;
		m_LastSavedRecords.ForEach([&](const UUID& uuid, const long& value) {
			out << YAML::Key << std::to_string(uuid);
			out << YAML::Value << value;
		});
		out << YAML::EndMap;

		m_PathToUUIDCache.Clear();
		m_LastSavedRecords.Clear();

		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path path = pProject->CachePath();
		path.append("LastSaved.yaml");

		std::ofstream outStream{ path };
		outStream << out.c_str();
		outStream.close();
	}

	void EditorAssetDatabase::Update()
	{
		m_ImportedResources.ForEachClear([](const ImportedResource& resource) {
			ImportAsset(resource.Path.string(), resource.Resource);
			if (m_AsyncImportCallback) m_AsyncImportCallback(resource.Resource);
		});

		EditorAssetCallbacks::RunCallbacks();
	}

	bool EditorAssetDatabase::ImportJob(std::filesystem::path path)
	{
		std::filesystem::path filePath = path;
		std::filesystem::path extension = filePath.extension();
		std::filesystem::path fileName = filePath.filename().replace_extension("");

		std::string ext = extension.string();
		std::for_each(ext.begin(), ext.end(), [](char& c) { c = std::tolower(c); });

		LoaderModule* pModule = Game::GetGame().GetEngine()->GetLoaderModule(ext);
		if (!pModule)
		{
			// Not supperted!
			Debug::LogError("Failed to import file, asset type not supported!");
			return false;
		}

		Resource* pResource = pModule->Load(path.string());
		if (!pResource)
		{
			Debug::LogError("Failed to import file, the returned Resource is null!");
			return false;
		}

		m_ImportedResources.push_back({ pResource, path });
		return true;
	}

	void EditorAssetDatabase::ImportModuleAssets(const std::filesystem::path& path)
	{
		std::stringstream stream;
		stream << "Importing module assets at " << path << "...";
		Debug::LogInfo(stream.str());

		for (auto itor : std::filesystem::directory_iterator(path))
		{
			std::filesystem::path path = itor.path();
			const std::string pathString = ".\\" + path.string();
			if (!itor.is_directory())
			{
				/* Is it already imported? */
				/* FIXME: This lookup is slow! */
				if (FindAssetUUID(pathString)) continue;
				ImportAsset(pathString);
				continue;
			}
			ImportModuleAssets(path);
		}
	}
}
