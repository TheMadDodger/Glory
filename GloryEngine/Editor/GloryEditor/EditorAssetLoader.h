#pragma once
#include <ThreadManager.h>
#include <filesystem>

namespace Glory::Editor
{
	class EditorAssetLoader
	{
	private:
		EditorAssetLoader();
		virtual ~EditorAssetLoader();

	private:
		static void Start();
		static void Stop();
		static void LoadAll();

		static void ProcessDirectory(const std::string& path, bool recursive = true, const std::string& folderFilter = "");
		static void ProcessFile(const std::filesystem::path& filePath);
		static void RemoveDeletedAssets();
		static void Run();

	private:
		friend class ProjectSpace;
		friend class MainEditor;
		static Thread* m_pThread;
		static bool m_Exit;
	};
}
