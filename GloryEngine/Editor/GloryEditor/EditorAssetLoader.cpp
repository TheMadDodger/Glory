#include "EditorAssetLoader.h"
#include "ProjectSpace.h"
#include "EditorAssetDatabase.h"
#include <AssetDatabase.h>
#include <yaml-cpp/yaml.h>
#include <Engine.h>

namespace Glory::Editor
{
	Thread* EditorAssetLoader::m_pThread = nullptr;
	bool EditorAssetLoader::m_Exit = false;

	EditorAssetLoader::EditorAssetLoader() {}

	EditorAssetLoader::~EditorAssetLoader() {}

	void EditorAssetLoader::Start()
	{
		m_Exit = false;
		m_pThread = ThreadManager::Run(EditorAssetLoader::Run);
	}

	void EditorAssetLoader::Stop()
	{
		m_Exit = true;
		while(m_pThread != nullptr && !m_pThread->IsIdle()) {}
		m_pThread = nullptr;
	}

	void EditorAssetLoader::LoadAll()
	{
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		if (!pProject) return;

		std::filesystem::path assetPath = pProject->RootPath();
		assetPath.append("Assets");

		ProcessDirectory(assetPath.string(), true);
		//RemoveDeletedAssets();
		//
		//assetPath = "./Modules/";
		//ProcessDirectory(assetPath.string(), true, "\\Assets\\");
	}

	void EditorAssetLoader::ProcessDirectory(const std::string& path, bool recursive, const std::string& folderFilter)
	{
		if (!std::filesystem::is_directory(path)) return;

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (entry.is_directory())
			{
				if (recursive) ProcessDirectory(entry.path().string(), true, folderFilter);
				continue;
			}

			std::string pathString = entry.path().string();
			if (folderFilter != "" && pathString.find(folderFilter) == std::string::npos) continue;
			ProcessFile(entry.path());
		}
	}

	void EditorAssetLoader::ProcessFile(const std::filesystem::path& filePath)
	{
		// Check if file was saved
		if (!std::filesystem::exists(filePath) || filePath.extension() == "TMP") return;
		std::filesystem::file_time_type lastSaveTime = std::filesystem::last_write_time(filePath);
		long duration = (long)lastSaveTime.time_since_epoch().count();
		const UUID uuid = EditorAssetDatabase::FindAssetUUID(filePath.string());
		if (!uuid) return;
		long previousDuration = EditorAssetDatabase::GetLastSavedRecord(uuid);
		if (duration != previousDuration)
		{
			// Asset was updated
			EditorAssetDatabase::UpdateAsset(uuid, duration);
		}

		// Make the path relative to the asset/resource path!
		//ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		//if (!pProject) return;
		//std::filesystem::path assetsPath = pProject->RootPath();
		//assetsPath.append("Assets");
		//std::string relativePathToFile = filePath.lexically_relative(assetsPath).string();
		//bool relativePath = true;
		//if (relativePathToFile == "")
		//{
		//	relativePathToFile = filePath.string();
		//	relativePath = false;
		//}
		//
		//// Process meta file if it exists
		//if (std::filesystem::exists(metaFilePath))
		//{
		//	// Both file and meta file exists we need to check if it also exists in the database!
		//	ResourceMeta meta(metaFilePath.string(), ext.string());
		//	meta.Read();
		//	UUID uuid = meta.ID();
		//	if (AssetDatabase::AssetExists(uuid))
		//	{
		//		AssetLocation location;
		//		AssetDatabase::GetAssetLocation(uuid, location);
		//		if (relativePathToFile != location.Path)
		//		{
		//			EditorAssetDatabase::UpdateAssetPath(uuid, relativePathToFile, metaFilePath.string());
		//		}
		//
		//		// Check if file was saved
		//		std::filesystem::file_time_type lastSaveTime = std::filesystem::last_write_time(filePath);
		//		long duration = lastSaveTime.time_since_epoch().count();
		//		long previousDuration = EditorAssetDatabase::GetLastSavedRecord(uuid);
		//		if (duration != previousDuration)
		//		{
		//			// Asset was updated
		//			EditorAssetDatabase::UpdateAsset(uuid, duration);
		//		}
		//		return;
		//	}
		//
		//	EditorAssetDatabase::InsertAsset(relativePathToFile, meta);
		//	return;
		//}
		//
		//std::string extension = ext.string();
		//std::for_each(extension.begin(), extension.end(), [](char& c) { c = std::tolower(c); });
		//ResourceType* pResourceType = ResourceType::GetResourceType(extension);
		//uint32_t hash = 0;
		//
		//if (pResourceType != nullptr) hash = pResourceType->Hash();
		//
		//LoaderModule* pLoader = Game::GetGame().GetEngine()->GetLoaderModule(hash);
		//
		//// If the meta file does not exist but the asset exists in the database then we should regenerate the meta file
		//UUID uuid = AssetDatabase::GetAssetUUID(relativePathToFile);
		//if (uuid != 0)
		//{
		//	ResourceMeta meta;
		//	AssetDatabase::GetResourceMeta(uuid, meta);
		//	meta.Write(pLoader);
		//	return;
		//}
		//
		//// Generate a meta file
		//ResourceMeta meta(metaFilePath.string(), ext.string(), UUID(), hash);
		//meta.Write(pLoader, relativePath);
		//meta.Read();
		//
		//EditorAssetDatabase::InsertAsset(relativePathToFile, meta);
	}

	void EditorAssetLoader::RemoveDeletedAssets()
	{
		/* TODO */
		//std::vector<UUID> toDeleteAssets;
		//EditorAssetDatabase::ForEachAssetLocation([&](UUID uuid, const AssetLocation& assetLocation)
		//{
		//	std::filesystem::path path = Game::GetAssetPath();
		//	path.append(assetLocation.Path);
		//	if (std::filesystem::exists(path)) return;
		//	path = assetLocation.Path;
		//	if (std::filesystem::exists(path)) return;
		//	toDeleteAssets.push_back(uuid);
		//});
		//
		//for (size_t i = 0; i < toDeleteAssets.size(); i++)
		//{
		//	AssetDatabase::RemoveAsset(toDeleteAssets[i]);
		//}
	}

	void EditorAssetLoader::Run()
	{
		while (!m_Exit)
		{
			if (m_Exit) return;
			LoadAll();
		}
	}
}
