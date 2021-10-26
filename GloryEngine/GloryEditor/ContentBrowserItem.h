#pragma once
#include <vector>
#include <string>
#include <filesystem>

namespace Glory::Editor
{
	class ContentBrowserItem
	{
	public:
		ContentBrowserItem();
		ContentBrowserItem(const std::string& name, bool isFolder, ContentBrowserItem* pParent);
		virtual ~ContentBrowserItem();

		static ContentBrowserItem* GetSelectedFolder();

		bool HasParent();
		void Up();
		static void HistoryUp();
		static void HistoryDown();

		void Change(const std::string& name, bool isFolder);
		void Refresh();
		void RefreshSelected(ContentBrowserItem* pRoot);
		std::filesystem::path BuildPath();

		void DrawDirectoryBrowser();
		static void DrawFileBrowser(int iconSize);
		static void DrawCurrentPath();
		void DrawFileItem(int iconSize);
		bool IsValid();
		void SetOpen();

		void SortChildren();

	private:
		static void EraseExcessHistory();

	private:
		std::string m_Name;
		bool m_IsFolder;
		bool m_SetOpen;
		std::filesystem::path m_CachedPath;
		ContentBrowserItem* m_pParent;
		std::vector<ContentBrowserItem*> m_pChildren;

		static std::hash<std::string> m_PathHasher;
		static ContentBrowserItem* m_pSelectedFolder;
		static std::vector<ContentBrowserItem*> m_pHistory;
		static size_t m_HistoryIndex;
	};
}
