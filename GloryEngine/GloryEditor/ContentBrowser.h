#pragma once
#include "EditorWindow.h"
#include <ContentBrowserItem.h>

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
		void DirectoryBrowser();
		void FileBrowserMenu();
		void DrawPathControls();
		void DrawSearchBar();
		void FileBrowser();
		void RefreshContentBrowser();

	private:
		int m_I;
		static int m_IconSize;
		std::hash<std::string> m_Hasher;
		std::filesystem::path m_RootAssetPath;
		char m_SearchBuffer[100];

		bool m_Refresh;
		ContentBrowserItem* m_pRootItem;
		uint64_t m_pBackTexture;
		uint64_t m_pForwardTexture;
		uint64_t m_pUpTexture;
		uint64_t m_pRefreshTexture;
	};
}