#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <functional>

namespace Glory::Editor
{
	template<typename T>
	struct Dispatcher;

	class EditableEntity;

	class FileBrowserItem
	{
	public:
		GLORY_EDITOR_API static FileBrowserItem* GetSelectedFolder();
		GLORY_EDITOR_API void SetSelectedFolder(const std::filesystem::path& path);
		GLORY_EDITOR_API static void SetSelectedFolder(FileBrowserItem* pItem);

		GLORY_EDITOR_API bool HasParent();
		GLORY_EDITOR_API void Up();
		GLORY_EDITOR_API static void HistoryUp();
		GLORY_EDITOR_API static void HistoryDown();

		GLORY_EDITOR_API void Change(const std::string& name, bool isFolder);
		GLORY_EDITOR_API void Refresh();
		GLORY_EDITOR_API void RefreshSelected(FileBrowserItem* pRoot);
		GLORY_EDITOR_API std::filesystem::path BuildPath();

		GLORY_EDITOR_API void DrawDirectoryBrowser();
		GLORY_EDITOR_API void DrawFileItem(int iconSize);
		GLORY_EDITOR_API bool IsValid();
		GLORY_EDITOR_API void SetOpen();

		GLORY_EDITOR_API void SortChildren();

		GLORY_EDITOR_API static std::filesystem::path GetCurrentPath();

		GLORY_EDITOR_API FileBrowserItem* GetChildByName(const std::string& name, bool folder);
		GLORY_EDITOR_API void BeginRename();

		bool IsEditable() const;

		GLORY_EDITOR_API static const std::string& GetHighlightedPath();
		GLORY_EDITOR_API void AddIgnoreDirectory(const std::string& directory);
		GLORY_EDITOR_API void AddIgnoreDirectories(const std::vector<std::string>& directories);
		GLORY_EDITOR_API const std::string& Name();

		GLORY_EDITOR_API static void PerformSearch(const std::vector<FileBrowserItem*>& pRootItems);
		GLORY_EDITOR_API static void PerformSearch(FileBrowserItem* pItem);
		GLORY_EDITOR_API static void ClearSearch();

		struct ObjectDNDEvent
		{
			std::filesystem::path Path;
			EditableEntity* Object;
		};
		GLORY_EDITOR_API static Dispatcher<ObjectDNDEvent>& ObjectDNDEventDispatcher();

	private:
		FileBrowserItem();
		FileBrowserItem(const std::string& name, bool isFolder, FileBrowserItem* pParent, bool isEditable, const std::string& directoryFilter = "", std::function<std::filesystem::path()> rootPathFunc = DefaultRootPathFunc);
		virtual ~FileBrowserItem();

		static void EraseExcessHistory();
		static std::filesystem::path DefaultRootPathFunc();
		static void DrawFileBrowser(int iconSize);
		static void DrawCurrentPath();

		void DrawName(float padding);

		void Update();

	private:
		friend class FileBrowser;
		std::string m_Name;
		bool m_IsFolder;
		bool m_SetOpen;
		std::function<std::filesystem::path()> m_RootPathFunc;
		std::filesystem::path m_CachedPath;
		FileBrowserItem* m_pParent;
		std::vector<FileBrowserItem*> m_pChildren;
		std::vector<std::string> m_IgnoreDirectories;
		std::string m_DirectoryFilter;

		bool m_StartEditingName;
		bool m_EditingName;
		char m_NameBuffer[1000];
		bool m_Editable;
		bool m_AutoScrollHere;
		float m_HighlightTimer;
		float m_HighlightFade;

		static std::hash<std::string> m_PathHasher;
		static FileBrowserItem* m_pSelectedFolder;
		static std::vector<FileBrowserItem*> m_pHistory;
		static size_t m_HistoryIndex;
		static std::string m_HighlightedPath;
		static char m_SearchBuffer[1000];
		static std::vector<FileBrowserItem*> m_pSearchResultCache;
		static bool m_Dirty;

		static Dispatcher<FileBrowserItem::ObjectDNDEvent>* m_pObjectDNDDispatcher;
	};
}
