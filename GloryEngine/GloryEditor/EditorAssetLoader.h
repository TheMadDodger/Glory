#pragma once
#include <ThreadManager.h>
#include <filesystem>

namespace Glory
{
	class EditorAssetLoader
	{
	public:
		EditorAssetLoader();
		virtual ~EditorAssetLoader();

	private:
		void Initialize();
		void Run();

		void ProcessDirectory(const std::string& path);
		void ProcessFile(const std::filesystem::path& filePath);

	private:
		friend class MainEditor;
		Thread* m_pThread;

	};
}
