#pragma once
#include "GloryEditor.h"

#include <filesystem>
#include <efsw/efsw.hpp>
#include <string_view>
#include <UUID.h>

namespace Glory::Editor
{
	template<typename Arg>
	struct Dispatcher;

	struct AssetsFileWatchEvent
	{
		const std::string Directory;
		const std::string Filename;
		const std::string OldFilename;
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

		GLORY_EDITOR_API static void RunCallbacks();

	private:
		static void HandleFileWatchInternal(const AssetsFileWatchEvent& e);

		static void ProcessDirectory(const std::string& path, bool recursive = true, const std::string& folderFilter = "");
		static void ProcessFileChange(const std::filesystem::path& filePath);
		static void RemoveDeletedAssets();

	private:
		friend class ProjectSpace;
		friend class MainEditor;

		long m_WatchID;

		static UUID m_InternalWatchHandler;
	};
}
