#pragma once
#include <ThreadManager.h>
#include <filesystem>
#include <efsw/efsw.hpp>

namespace Glory::Editor
{
	class EditorAssetsWatcher : public efsw::FileWatchListener
	{
	public:
		EditorAssetsWatcher();
		virtual ~EditorAssetsWatcher();

		void handleFileAction(efsw::WatchID watchid, const std::string& dir,
			const std::string& filename, efsw::Action action, std::string oldFilename) override;

	private:
		void ProcessDirectory(const std::string& path, bool recursive = true, const std::string& folderFilter = "");
		void ProcessFileChange(const std::filesystem::path& filePath);
		void RemoveDeletedAssets();

	private:
		friend class ProjectSpace;
		friend class MainEditor;

		long m_WatchID;
	};
}
