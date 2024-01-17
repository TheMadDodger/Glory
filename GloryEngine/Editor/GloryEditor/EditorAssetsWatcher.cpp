#include "EditorAssetsWatcher.h"
#include "ProjectSpace.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"
#include "Dispatcher.h"

#include <Engine.h>

namespace Glory::Editor
{
	UUID EditorAssetsWatcher::m_InternalWatchHandler = 0;

	void EditorAssetsWatcher::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename)
	{
		AssetsFileWatchEvents().Enqueue(AssetsFileWatchEvent{ dir.data(), filename.data(), oldFilename.data(), action });
	}

	EditorAssetsWatcher::AssetsFileWatchDispatcher& EditorAssetsWatcher::AssetsFileWatchEvents()
	{
		static AssetsFileWatchDispatcher dispatcher;
		return dispatcher;
	}

	void EditorAssetsWatcher::RunCallbacks()
	{
		AssetsFileWatchEvents().Flush();
	}

	EditorAssetsWatcher::EditorAssetsWatcher() : m_WatchID(0)
	{
		ProjectSpace::RegisterCallback(ProjectCallback::OnOpen, [&](ProjectSpace* pProject) {
			std::filesystem::path assetPath = pProject->RootPath();
			assetPath.append("Assets");
			m_WatchID = EditorApplication::GetInstance()->FileWatch().addWatch(assetPath.string(), this, true);
			});

		ProjectSpace::RegisterCallback(ProjectCallback::OnClose, [&](ProjectSpace* pProject) {
			EditorApplication::GetInstance()->FileWatch().removeWatch(m_WatchID);
		});

		m_InternalWatchHandler = AssetsFileWatchEvents().AddListener(HandleFileWatchInternal);
	}

	EditorAssetsWatcher::~EditorAssetsWatcher()
	{
		AssetsFileWatchEvents().RemoveListener(m_InternalWatchHandler);
	}

	void EditorAssetsWatcher::HandleFileWatchInternal(const AssetsFileWatchEvent& e)
	{
		std::filesystem::path filePath = e.Directory;
		filePath.append(e.Filename);

		switch (e.Action)
		{
		case efsw::Actions::Add:
			/* TODO: Process externally added assets */
			//std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Added" << std::endl;
			break;
		case efsw::Actions::Delete:
			/* TODO: Process externally deleted assets */
			//std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Delete" << std::endl;
			break;
		case efsw::Actions::Modified:
			//std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Modified" << std::endl;
			ProcessFileChange(filePath);
			break;
		case efsw::Actions::Moved:
			/* TODO: Process externally moved assets */
			//std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Moved from (" << oldFilename << ")" << std::endl;
			break;
			//default:
				//std::cout << "Should never happen!" << std::endl;
		}
	}

	void EditorAssetsWatcher::ProcessDirectory(const std::string& path, bool recursive, const std::string& folderFilter)
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
			ProcessFileChange(entry.path());
		}
	}

	void EditorAssetsWatcher::ProcessFileChange(const std::filesystem::path& filePath)
	{
		if (!std::filesystem::exists(filePath)) return;

		/* TODO: Check for external changes in directories */
		if (std::filesystem::is_directory(filePath)) return;

		const UUID uuid = EditorAssetDatabase::FindAssetUUID(filePath.string());
		if (!uuid) return;
		// Asset was updated
		EditorAssetDatabase::UpdateAsset(uuid);

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

	void EditorAssetsWatcher::RemoveDeletedAssets()
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
}
