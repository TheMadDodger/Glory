#include "EditorAssetDatabase.h"
#include "AssetManager.h"
#include "EditorAssetCallbacks.h"
#include "ProjectSpace.h"
#include "FileBrowser.h"
#include "EditorSceneManager.h"
#include "AssetCompiler.h"
#include "Importer.h"
#include "EditorApplication.h"
#include "AssetDatabase.h"
#include "EditorResourceManager.h"
#include "EditableResource.h"

#include <JobManager.h>
#include <Engine.h>
#include <Debug.h>

namespace Glory::Editor
{
	ThreadedVector<UUID> m_UnsavedAssets;

	Engine* DB_EngineInstance = nullptr;

	struct ImportedResource
	{
		Resource* Resource;
		std::filesystem::path Path;
	};

	ThreadedVector<ImportedResource> m_ImportedResources;
	ThreadedUMap<std::string, UUID> m_PathToUUIDCache;

	std::function<void(Resource*)> EditorAssetDatabase::m_AsyncImportCallback;
	bool EditorAssetDatabase::m_IsDirty;

	Jobs::JobPool<bool, std::filesystem::path>* m_pImportPool;

	void EditorAssetDatabase::Load(JSONFileRef& projectFile)
	{
		JSONValueRef assetsNode = projectFile["Assets"];
		if (!projectFile["Assets"].Exists() || !projectFile["Assets"].IsObject())
		{
			projectFile["Assets"].SetObject();
		}

		m_PathToUUIDCache.Clear();
		for (const auto& f : assetsNode)
		{
			const std::string_view key = f.name.GetString();
			const UUID uuid = std::stoull(key.data());
			EditorAssetCallbacks::EnqueueCallback(AssetCallbackType::CT_AssetRegistered, uuid, nullptr);

			JSONValueRef assetNode = assetsNode[key];
			JSONValueRef locationNode = assetNode["Location"];
			JSONValueRef pathNode = locationNode["Path"];
			JSONValueRef subPathNode = locationNode["SubresourcePath"];
			JSONValueRef indexNode = locationNode["Index"];

			if (!subPathNode.AsString().empty()) continue;

			std::string_view path = pathNode.AsString();
			std::filesystem::path absolutePath = path;
			if (!absolutePath.is_absolute() && path[0] != '.')
			{
				absolutePath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
				absolutePath = absolutePath.append(path);
			}

			m_PathToUUIDCache.Set(absolutePath.string(), uuid);
		}

		DB_EngineInstance->GetDebug().LogInfo("Loaded asset database");
		AssetCompiler::CompileAssetDatabase();
		AssetCompiler::CompileNewAssets();
	}

	void EditorAssetDatabase::Reload()
	{
		DB_EngineInstance->GetAssetDatabase().Clear();
		AssetCompiler::CompileAssetDatabase();

		DB_EngineInstance->GetDebug().LogInfo("Reloaded asset database");
	}

	void EditorAssetDatabase::InsertAsset(AssetLocation& location, const ResourceMeta& meta, bool setDirty)
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();

		std::replace(location.Path.begin(), location.Path.end(), '/', '\\');
		uint64_t uuid = meta.ID();
		const std::string key = std::to_string(uuid);
		JSONValueRef assetNode = projectFile["Assets"][key];
		/* We are not updating the asset but adding it */
		/* So if the asset already exists we ignore it */
		if (assetNode.Exists() && assetNode.IsObject()) return;

		JSONValueRef locationNode = assetNode["Location"];
		locationNode["Path"].SetString(location.Path);
		locationNode["SubresourcePath"].SetString(location.SubresourcePath);
		locationNode["Index"].SetInt(int(location.Index));

		JSONValueRef metaNode = assetNode["Metadata"];
		metaNode["Extension"].SetString(meta.Extension());
		metaNode["Name"].SetString(meta.Name());
		metaNode["UUID"].SetUInt64(meta.ID());
		metaNode["Hash"].SetUInt(meta.Hash());
		metaNode["SerializedVersion"].SetUInt64(meta.SerializedVersion());

		EditorAssetCallbacks::EnqueueCallback(AssetCallbackType::CT_AssetRegistered, uuid, nullptr);
		SetDirty();
	}

	void EditorAssetDatabase::UpdateAssetPath(UUID uuid, const std::string& newPath)
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetNode = projectFile["Assets"][std::to_string(uuid)];

		if (!assetNode.Exists() || !assetNode.IsObject()) return;

		std::string fixedNewPath = newPath;
		std::replace(fixedNewPath.begin(), fixedNewPath.end(), '/', '\\');
		const std::string_view oldPath = assetNode["Location"]["Path"].AsString();
		assetNode["Location"]["Path"].SetString(fixedNewPath);
		assetNode["Metadata"]["Name"].SetString(std::filesystem::path{newPath}.filename().replace_extension().string());

		std::filesystem::path oldAbsolutePath = oldPath;
		if (!oldAbsolutePath.is_absolute() && oldPath[0] != '.')
		{
			oldAbsolutePath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
			oldAbsolutePath = oldAbsolutePath.append(oldPath);
		}
		std::filesystem::path newAbsolutePath = newPath;
		if (!newAbsolutePath.is_absolute() && newPath[0] != '.')
		{
			newAbsolutePath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
			newAbsolutePath = newAbsolutePath.append(newPath);
		}
		m_PathToUUIDCache.Erase(oldAbsolutePath.string());
		m_PathToUUIDCache.Set(newAbsolutePath.string(), uuid);

		std::stringstream stream;
		stream << "Moved asset from " << oldPath << " to " << fixedNewPath;
		DB_EngineInstance->GetDebug().LogInfo(stream.str());

		AssetCompiler::CompileAssetDatabase(uuid);
		EditorApplication::GetInstance()->GetResourceManager().ReloadEditableAsset(uuid);

		SetDirty();
	}

	void EditorAssetDatabase::UpdateAsset(UUID uuid)
	{
		EditorAssetCallbacks::EnqueueCallback(AssetCallbackType::CT_AssetUpdated, uuid, nullptr);
	}

	void EditorAssetDatabase::UpdateAssetPaths(const std::string& oldPath, const std::string& newPath)
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];

		std::string fixedNewPath = newPath;
		std::replace(fixedNewPath.begin(), fixedNewPath.end(), '/', '\\');
		std::string fixedOldPath = oldPath;
		std::replace(fixedOldPath.begin(), fixedOldPath.end(), '/', '\\');

		std::filesystem::path absolutePath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
		absolutePath = absolutePath.append(fixedOldPath);

		/* Find all assets inside this folder and update their folder */
		for (const auto& f : assetsNode)
		{
			const std::string_view key = f.name.GetString();
			UUID uuid = std::stoull(key.data());
			JSONValueRef assetNode = assetsNode[key];
			const std::string_view pathStr = assetNode["Location"]["Path"].AsString();
			std::filesystem::path absoluteAssetPath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
			absoluteAssetPath = absoluteAssetPath.append(pathStr);
			if (absoluteAssetPath.string().find(absolutePath.string()) == std::string::npos) continue;

			std::string path = std::string(pathStr);
			size_t index = path.find(fixedOldPath);
			size_t length = fixedOldPath.length();
			path = path.replace(index, length, fixedNewPath);
			UpdateAssetPath(uuid, path);
		}
	}

	void EditorAssetDatabase::DeleteAsset(UUID uuid, bool compile)
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];

		const std::string key = std::to_string(uuid);
		JSONValueRef assetNode = assetsNode[key];
		if (!assetNode.Exists() || !assetNode.IsObject()) return;

		assetsNode.Remove(key);
		EditorAssetCallbacks::EnqueueCallback(AssetCallbackType::CT_AssetDeleted, uuid, nullptr);

		std::stringstream stream;
		stream << "Deleted asset: " << uuid;
		DB_EngineInstance->GetDebug().LogInfo(stream.str());

		if (compile) AssetCompiler::CompileAssetDatabase(uuid);

		SetDirty();
	}

	void EditorAssetDatabase::DeleteAsset(const std::string& path)
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];

		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');

		std::filesystem::path absolutePath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
		absolutePath = absolutePath.append(fixedPath);

		/* Find all assets on this path */
		std::vector<UUID> relevantAssets;
		for (const auto& f : assetsNode)
		{
			const std::string_view key = f.name.GetString();
			const UUID uuid = std::stoull(key.data());
			JSONValueRef assetNode = assetsNode[key];
			const std::string_view pathStr = assetNode["Location"]["Path"].AsString();
			std::filesystem::path absoluteAssetPath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
			absoluteAssetPath = absoluteAssetPath.append(pathStr);
			if (absoluteAssetPath != absolutePath.string()) continue;
			relevantAssets.push_back(uuid);
		}

		for (size_t i = 0; i < relevantAssets.size(); i++)
		{
			UUID uuid = relevantAssets[i];
			DeleteAsset(uuid, false);
		}

		AssetCompiler::CompileAssetDatabase(relevantAssets);
	}

	void EditorAssetDatabase::DeleteAssets(const std::string& path)
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];

		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');

		std::filesystem::path absolutePath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
		absolutePath = absolutePath.append(fixedPath);

		std::vector<UUID> relevantAssets;
		for (const auto& f : assetsNode)
		{
			const std::string_view key = f.name.GetString();
			const UUID uuid = std::stoull(key.data());
			JSONValueRef assetNode = assetsNode[key];
			const std::string_view pathStr = assetNode["Location"]["Path"].AsString();
			std::filesystem::path absoluteAssetPath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
			absoluteAssetPath = absoluteAssetPath.append(pathStr);
			if (absoluteAssetPath.string().find(absolutePath.string()) == std::string::npos) continue;
			relevantAssets.push_back(uuid);
		}

		for (size_t i = 0; i < relevantAssets.size(); i++)
		{
			UUID uuid = relevantAssets[i];
			DeleteAsset(uuid, false);
		}

		AssetCompiler::CompileAssetDatabase(relevantAssets);
	}

	void EditorAssetDatabase::IncrementAssetVersion(UUID uuid)
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];

		JSONValueRef assetNode = assetsNode[std::to_string(uuid)];
		if (!assetNode.Exists() || !assetNode.IsObject()) return;
		JSONValueRef metaNode = assetNode["Metadata"];

		const uint64_t version = metaNode["SerializedVersion"].AsUInt64() + 1;
		metaNode["SerializedVersion"].SetUInt64(version);

		AssetCompiler::CompileAssetDatabase(uuid);

		SetDirty();
	}

	UUID EditorAssetDatabase::CreateAsset(Resource* pResource, const std::string& path)
	{
		std::filesystem::path filePath = path;
		std::filesystem::path extension = filePath.extension();
		std::filesystem::path fileName = filePath.filename().replace_extension("");
		if (!Importer::Export(path, pResource)) return 0;
		return ImportAsset(path, pResource);
	}

	UUID EditorAssetDatabase::ImportAsset(const std::string& path, Resource* pLoadedResource, std::filesystem::path subPath)
	{
		std::filesystem::path filePath = path;
		std::filesystem::path extension = filePath.extension();
		std::filesystem::path fileName = filePath.filename().replace_extension("");

		std::string ext = extension.string();
		std::for_each(ext.begin(), ext.end(), [](char& c) { c = std::tolower(c); });

		if (DB_EngineInstance->GetResourceTypes().IsScene(ext))
		{
			ImportScene(path);
			return 0;
		}

		const ResourceType* pType = nullptr;
		if (!pLoadedResource)
		{
			pLoadedResource = Importer::Import(path, nullptr);
			if (!pLoadedResource)
			{
				DB_EngineInstance->GetDebug().LogWarning("Failed to import file using new importer system, using legacy loaders instead!");

				LoaderModule* pModule = DB_EngineInstance->GetLoaderModule(ext);
				if (!pModule)
				{
					// Not supperted!
					DB_EngineInstance->GetDebug().LogError("Failed to import file, asset type not supported!");
					return 0;
				}

				pLoadedResource = pModule->Load(path);

				if (!pLoadedResource)
				{
					DB_EngineInstance->GetDebug().LogError("Failed to import file, could not load resource file!");
					return 0;
				}
			}
		}

		/* Try getting the resource type from the loaded resource */
		std::type_index type = typeid(Resource);
		for (size_t i = 0; i < pLoadedResource->TypeCount(); ++i)
		{
			if (!pLoadedResource->GetType(0, type)) continue;
			pType = DB_EngineInstance->GetResourceTypes().GetResourceType(type);
			if (pType) break;
		}

		if (!pType)
		{
			/* Try to get the resource type from the extension */
			pType = DB_EngineInstance->GetResourceTypes().GetResourceType(ext);

			if (!pType)
			{
				// Not supperted!
				DB_EngineInstance->GetDebug().LogError("Failed to import file, could not determine ResourceType!");
				return 0;
			}
		}

		// Generate a meta file
		AssetDatabase& assetDatabase = DB_EngineInstance->GetAssetDatabase();
		AssetManager& assetManager = DB_EngineInstance->GetAssetManager();
		const std::string_view assetPath = assetDatabase.GetAssetPath();

		std::filesystem::path namePath = fileName;
		if (!subPath.empty()) namePath.append(subPath.string());
		ResourceMeta meta(extension.string(), namePath.string(), pLoadedResource->GetUUID(), pType->Hash());

		assetDatabase.SetIDAndName(pLoadedResource, meta.ID(), meta.Name());
		std::filesystem::path relativePath = filePath.lexically_relative(assetPath);
		assetManager.AddLoadedResource(pLoadedResource);

		AssetLocation location{ relativePath.empty() ? path : relativePath.string(), subPath.string() };
		InsertAsset(location, meta);
		const UUID uuid = meta.ID();
		if (subPath.empty()) m_PathToUUIDCache.Set(path, uuid);

		std::stringstream stream;
		if (!subPath.empty())
			stream << "Imported subasset " << subPath.string() << " at " << path;
		else stream << "Imported asset at " << path;
		DB_EngineInstance->GetDebug().LogInfo(stream.str());

		AssetCompiler::CompileAssetDatabase(meta.ID());

		/* Import sub resources */
		for (size_t i = 0; i < pLoadedResource->SubResourceCount(); i++)
		{
			Resource* pSubResource = pLoadedResource->Subresource(i);
			std::filesystem::path newSubPath = subPath;
			newSubPath.append(pSubResource->Name());
			ImportAsset(path, pSubResource, newSubPath);
		}

		return meta.ID();
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
		DB_EngineInstance->GetDebug().LogInfo(stream.str());
		m_pImportPool->QueueSingleJob(ImportJob, path);
	}

	void EditorAssetDatabase::ImportNewScene(const std::string& path, GScene* pScene)
	{
		std::stringstream stream;
		stream << "Importing new scene at " << path << "...";
		DB_EngineInstance->GetDebug().LogInfo(stream.str());

		std::filesystem::path filePath = path;
		std::filesystem::path extension = filePath.extension();
		std::filesystem::path fileName = filePath.filename().replace_extension("");
		std::filesystem::path metaExtension = std::filesystem::path(".gmeta");

		// Generate a meta
		const ResourceType* pType = DB_EngineInstance->GetResourceTypes().GetResourceType<GScene>();

		const std::string_view assetPath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
		ResourceMeta meta(extension.string(), fileName.string(), pScene->GetUUID(), pType->Hash());

		AssetDatabase& assetDatabase = DB_EngineInstance->GetAssetDatabase();
		AssetManager& assetManager = DB_EngineInstance->GetAssetManager();
		assetDatabase.SetIDAndName(pScene, meta.ID(), fileName.string());
		std::filesystem::path relativePath = filePath.lexically_relative(assetDatabase.GetAssetPath());
		assetManager.AddLoadedResource(pScene);
		AssetLocation location{ relativePath.string() };
		InsertAsset(location, meta);
		m_PathToUUIDCache.Set(path, meta.ID());

		stream.clear();
		stream << "Imported new scene: " << pScene->Name();
		DB_EngineInstance->GetDebug().LogInfo(stream.str());

		AssetCompiler::CompileAssetDatabase(meta.ID());
	}

	void EditorAssetDatabase::ImportScene(const std::string& path)
	{
		std::stringstream stream;
		stream << "Importing scene at " << path << "...";
		DB_EngineInstance->GetDebug().LogInfo(stream.str());

		std::filesystem::path filePath = path;
		std::filesystem::path extension = filePath.extension();
		std::filesystem::path fileName = filePath.filename().replace_extension("");
		std::filesystem::path metaExtension = std::filesystem::path(".gmeta");

		// Generate a meta
		const ResourceType* pType = DB_EngineInstance->GetResourceTypes().GetResourceType<GScene>();

		const std::string_view assetPath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
		ResourceMeta meta(extension.string(), fileName.string(), UUID(), pType->Hash());

		std::filesystem::path relativePath = filePath.lexically_relative(assetPath);
		AssetLocation location{ relativePath.string() };
		InsertAsset(location, meta);
		m_PathToUUIDCache.Set(path, meta.ID());

		stream.clear();
		stream << "Imported scene: " << GetAssetName(meta.ID());
		DB_EngineInstance->GetDebug().LogInfo(stream.str());

		AssetCompiler::CompileAssetDatabase(meta.ID());
	}

	void EditorAssetDatabase::SaveAsset(Resource* pResource, bool markUndirty)
	{
		if (!pResource) return;

		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];

		const UUID uuid = pResource->GetUUID();
		const std::string key = std::to_string(uuid);
		JSONValueRef assetNode = assetsNode[key];

		if (!assetNode.Exists() || !assetNode.IsObject()) return;

		JSONValueRef location = assetNode["Location"];
		JSONValueRef meta = assetNode["Metadata"];

		std::filesystem::path path = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
		path.append(location["Path"].AsString());
		if (!Importer::Export(path, pResource)) return;
		IncrementAssetVersion(pResource->GetUUID());

		if (markUndirty)
			m_UnsavedAssets.Erase(pResource->GetUUID());

		std::stringstream stream;
		stream << "Saved asset to " << location["Path"].AsString();
		DB_EngineInstance->GetDebug().LogInfo(stream.str());

		AssetCompiler::CompileAssetDatabase(pResource->GetUUID());
	}

	void EditorAssetDatabase::RemoveAsset(UUID uuid)
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];

		const std::string key = std::to_string(uuid);
		JSONValueRef assetNode = assetsNode[key];
		if (!assetNode.Exists() || !assetNode.IsObject()) return;
		assetsNode.Remove(key);
		SetDirty();

		std::stringstream stream;
		stream << "Removed asset " << uuid;
		DB_EngineInstance->GetDebug().LogInfo(stream.str());

		AssetCompiler::CompileAssetDatabase(uuid);
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
			EditableResource* pResource = EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(uuid);
			if (!pResource) return;
			pResource->Save();
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

	std::vector<UUID> EditorAssetDatabase::UUIDs()
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];
		return assetsNode.IDKeys();
	}

	bool EditorAssetDatabase::GetAssetLocation(UUID uuid, AssetLocation& location)
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];

		const std::string key = std::to_string(uuid);
		JSONValueRef assetNode = assetsNode[key];
		if (!assetNode.Exists() || !assetNode.IsObject()) return false;
		JSONValueRef locationNode = assetNode["Location"];
		location.Path = locationNode["Path"].AsString();
		location.SubresourcePath = locationNode["SubresourcePath"].AsString();
		location.Index = locationNode["Index"].AsUInt64();
		return true;
	}

	bool EditorAssetDatabase::GetAssetMetadata(UUID uuid, ResourceMeta& meta)
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];

		const std::string key = std::to_string(uuid);
		JSONValueRef assetNode = assetsNode[key];
		if (!assetNode.Exists() || !assetNode.IsObject()) return false;

		JSONValueRef metaNode = assetNode["Metadata"];

		const std::string_view ext = metaNode["Extension"].AsString();
		const std::string_view name = metaNode["Name"].AsString();
		const uint32_t hash = metaNode["Hash"].AsUInt();
		const uint64_t version = metaNode["SerializedVersion"].AsUInt64();

		meta = ResourceMeta(std::string(ext), std::string(name), uuid, hash, version);
		return true;
	}

	UUID EditorAssetDatabase::FindAssetUUID(const std::string& path)
	{
		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '/', '\\');

		std::filesystem::path absolutePath = fixedPath;
		if (!absolutePath.is_absolute() && fixedPath[0] != '.')
		{
			absolutePath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
			absolutePath = absolutePath.append(fixedPath);
		}

		if (!m_PathToUUIDCache.Contains(absolutePath.string())) return 0;
		return m_PathToUUIDCache[absolutePath.string()];

		//for (YAML::const_iterator itor = m_Database.begin(); itor != m_Database.end(); ++itor)
		//{
		//	UUID uuid = itor->first.as<uint64_t>();
		//	const std::string key = std::to_string(uuid);
		//	YAML::Node assetNode = m_Database[key];
		//
		//	const AssetLocation location = assetNode["Location"].as<AssetLocation>();
		//	std::filesystem::path absoluteAssetPath = location.Path;
		//	if (!absoluteAssetPath.is_absolute() && location.Path[0] != '.')
		//	{
		//		absoluteAssetPath = DB_EngineInstance->GetAssetDatabase().GetAssetPath();
		//		absoluteAssetPath.append(location.Path);
		//	}
		//	if (absoluteAssetPath.string().find(absolutePath.string()) == std::string::npos) continue;
		//	return uuid;
		//}

		return 0;
	}

	bool EditorAssetDatabase::AssetExists(UUID uuid)
	{
		JSONFileRef & projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];

		const std::string key = std::to_string(uuid);
		JSONValueRef assetNode = assetsNode[key];
		return assetNode.Exists() && assetNode.IsObject();
	}

	std::string EditorAssetDatabase::GetAssetName(UUID uuid)
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];

		const std::string key = std::to_string(uuid);
		JSONValueRef assetNode = assetsNode[key];
		if (!assetNode.Exists() || !assetNode.IsObject()) return "";
		const std::string_view name = assetNode["Metadata/Name"].AsString();
		if (!name.empty()) return std::string(name);
		const std::string_view location = assetNode["Location/Path"].AsString();
		const std::string_view subPath = assetNode["Location/SubresourcePath"].AsString();
		std::filesystem::path fileName = std::filesystem::path(location).filename().replace_extension();
		if (!subPath.empty()) fileName.append(subPath);
		return fileName.string();
	}

	void EditorAssetDatabase::GetAllAssetsOfType(uint32_t typeHash, std::vector<UUID>& result)
	{
		JSONFileRef& projectFile = ProjectSpace::GetOpenProject()->ProjectFile();
		JSONValueRef assetsNode = projectFile["Assets"];

		for (const auto& f : assetsNode)
		{
			const std::string_view key = f.name.GetString();
			const UUID uuid = std::stoull(key.data());
			JSONValueRef assetNode = assetsNode[key];

			const uint32_t hash = assetNode["Metadata"]["Hash"].AsUInt();
			if (hash != typeHash) continue;
			result.push_back(uuid);
		}
	}

	void EditorAssetDatabase::RegisterAsyncImportCallback(std::function<void(Resource*)> func)
	{
		m_AsyncImportCallback = func;
	}

	void EditorAssetDatabase::ImportModuleAssets()
	{
		for (size_t i = 0; i < DB_EngineInstance->ModulesCount(); i++)
		{
			Module* pModule = DB_EngineInstance->GetModule(i);
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
		DB_EngineInstance = EditorApplication::GetInstance()->GetEngine();

		m_pImportPool = Jobs::JobManager::Run<bool, std::filesystem::path>();

		YAML::Node m_LastSavedNode;
		DB_EngineInstance->GetDebug().LogInfo("Initialized EditorAssetDatabase");
	}

	void EditorAssetDatabase::Cleanup()
	{
		DB_EngineInstance->GetDebug().LogInfo("Cleanup EditorAssetDatabase");

		m_pImportPool = nullptr;
		m_AsyncImportCallback = NULL;

		m_PathToUUIDCache.Clear();
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

		Resource* pResource = Importer::Import(path, nullptr);
		if (!pResource)
		{
			DB_EngineInstance->GetDebug().LogWarning("Failed to import file using new importer system, using legacy loaders instead!");

			LoaderModule* pModule = DB_EngineInstance->GetLoaderModule(ext);
			if (!pModule)
			{
				// Not supperted!
				DB_EngineInstance->GetDebug().LogError("Failed to import file, asset type not supported!");
				return false;
			}

			pResource = pModule->Load(path.string());
			if (!pResource)
			{
				DB_EngineInstance->GetDebug().LogError("Failed to import file, the returned Resource is null!");
				return false;
			}
		}

		m_ImportedResources.push_back({ pResource, path });
		return true;
	}

	void EditorAssetDatabase::ImportModuleAssets(const std::filesystem::path& path)
	{
		std::stringstream stream;
		stream << "Importing module assets at " << path << "...";
		DB_EngineInstance->GetDebug().LogInfo(stream.str());

		for (auto itor : std::filesystem::directory_iterator(path))
		{
			std::filesystem::path path = itor.path();
			const std::string pathString = path.string();
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
