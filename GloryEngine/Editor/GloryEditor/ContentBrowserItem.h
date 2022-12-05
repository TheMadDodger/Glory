#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <functional>

namespace Glory::Editor
{
	class ContentBrowserItem
	{
	public:
		static GLORY_EDITOR_API ContentBrowserItem* GetSelectedFolder();
		static GLORY_EDITOR_API void SetSelectedFolder(ContentBrowserItem* pItem);

		GLORY_EDITOR_API bool HasParent();
		GLORY_EDITOR_API void Up();
		static GLORY_EDITOR_API void HistoryUp();
		static GLORY_EDITOR_API void HistoryDown();

		GLORY_EDITOR_API void Change(const std::string& name, bool isFolder);
		GLORY_EDITOR_API void Refresh();
		GLORY_EDITOR_API void RefreshSelected(ContentBrowserItem* pRoot);
		GLORY_EDITOR_API std::filesystem::path BuildPath();

		GLORY_EDITOR_API void DrawDirectoryBrowser();
		GLORY_EDITOR_API void DrawFileItem(int iconSize);
		GLORY_EDITOR_API bool IsValid();
		GLORY_EDITOR_API void SetOpen();

		GLORY_EDITOR_API void SortChildren();

		static GLORY_EDITOR_API std::filesystem::path GetCurrentPath();

		GLORY_EDITOR_API ContentBrowserItem* GetChildByName(const std::string& name, bool folder);
		GLORY_EDITOR_API void BeginRename();

		bool IsEditable() const;

		static GLORY_EDITOR_API const std::string& GetHighlightedPath();
		GLORY_EDITOR_API void AddIgnoreDirectory(const std::string& directory);
		GLORY_EDITOR_API void AddIgnoreDirectories(const std::vector<std::string>& directories);

	private:
		ContentBrowserItem();
		ContentBrowserItem(const std::string& name, bool isFolder, ContentBrowserItem* pParent, bool isEditable, const std::string& directoryFilter = "", std::function<std::filesystem::path()> rootPathFunc = DefaultRootPathFunc);
		virtual ~ContentBrowserItem();

		static void EraseExcessHistory();
		static std::filesystem::path DefaultRootPathFunc();
		static void DrawFileBrowser(int iconSize);
		static void DrawCurrentPath();

		void DrawName();

	private:
		friend class ContentBrowser;
		std::string m_Name;
		bool m_IsFolder;
		bool m_SetOpen;
		std::function<std::filesystem::path()> m_RootPathFunc;
		std::filesystem::path m_CachedPath;
		ContentBrowserItem* m_pParent;
		std::vector<ContentBrowserItem*> m_pChildren;
		std::vector<std::string> m_IgnoreDirectories;
		std::string m_DirectoryFilter;

		bool m_StartEditingName;
		bool m_EditingName;
		char m_NameBuffer[1000];
		bool m_Editable;

		static std::hash<std::string> m_PathHasher;
		static ContentBrowserItem* m_pSelectedFolder;
		static std::vector<ContentBrowserItem*> m_pHistory;
		static size_t m_HistoryIndex;
		static std::string m_HighlightedPath;
	};
}
