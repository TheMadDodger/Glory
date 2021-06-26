#pragma once
#include "EditorWindow.h"
#include <filesystem>

namespace Glory::Editor
{
	class ContentBrowser : public EditorWindowTemplate<ContentBrowser>
	{
	public:
		ContentBrowser();
		virtual ~ContentBrowser();

	private:
		virtual void OnGUI() override;

	private:
		void ProcessDirectory(std::filesystem::path path);
		void ProcessFile(std::filesystem::path path);

		void DirectoryBrowser();
		void FileBrowser();
		void NonMetaFile(std::filesystem::path& path);

	private:
		int m_I;
		static int m_IconSize;
		std::hash<std::string> m_Hasher;
		std::filesystem::path m_SelectedPath;
		std::filesystem::path m_RootAssetPath;
	};
}