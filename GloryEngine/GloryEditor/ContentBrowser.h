#pragma once
#include "EditorWindow.h"
#include <ContentBrowserItem.h>
#include <Resource.h>

namespace Glory::Editor
{
	class ContentBrowser : public EditorWindowTemplate<ContentBrowser>
	{
	public:
		ContentBrowser();
		virtual ~ContentBrowser();

		static std::filesystem::path GetCurrentPath();

		virtual void OnOpen() override;
		virtual void OnClose() override;

		static void BeginRename(const std::string& name, bool folder);

		static void LoadProject();

	private:
		virtual void OnGUI() override;

	private:
		void DirectoryBrowser();
		void FileBrowserMenu();
		void DrawPathControls();
		void DrawSearchBar();
		void FileBrowser();
		void RefreshContentBrowser();

		void LoadItems();

	private:
		int m_I;
		static int m_IconSize;
		std::hash<std::string> m_Hasher;
		char m_SearchBuffer[100];

		std::vector<ContentBrowserItem*> m_pRootItems;
	};
}