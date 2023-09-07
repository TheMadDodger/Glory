#pragma once
#include "GloryEditor.h"

#include <filesystem>
#include <efsw/efsw.hpp>
#include <string_view>

namespace Glory::Editor
{
	template<typename Arg>
	struct Dispatcher;

	struct AssetsFileWatchEvent
	{
		const std::string_view Directory;
		const std::string_view Filename;
		const std::string_view OldFilename;
		const efsw::Action Action;
	};

	class EditorAssetsWatcher : public efsw::FileWatchListener
	{
	public:
		EditorAssetsWatcher();
		virtual ~EditorAssetsWatcher();

		void handleFileAction(efsw::WatchID watchid, const std::string& dir,
			const std::string& filename, efsw::Action action, std::string oldFilename) override;

		using AssetsFileWatchDispatcher = Dispatcher<AssetsFileWatchEvent>;

		GLORY_EDITOR_API static AssetsFileWatchDispatcher& AssetsFileWatchEvents();

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
