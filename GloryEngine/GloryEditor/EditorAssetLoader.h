#pragma once
#include <ThreadManager.h>
#include <filesystem>

namespace Glory::Editor
{
	class EditorAssetLoader
	{
	public:
		EditorAssetLoader();
		virtual ~EditorAssetLoader();

		void LoadAssets();

	private:
		void Initialize();
		void LoadAll();

		void ProcessDirectory(const std::string& path, bool recursive = true);
		void ProcessFile(const std::filesystem::path& filePath);

		void RemoveDeletedAssets();

	private:
		friend class MainEditor;
		Thread* m_pThread;
	};
}
