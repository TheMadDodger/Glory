#pragma once
#include "EditorWindow.h"
#include "FileBrowserItem.h"
#include <Resource.h>

namespace Glory::Editor
{
	class FileBrowser : public EditorWindowTemplate<FileBrowser>
	{
	public:
		FileBrowser();
		virtual ~FileBrowser();

		static GLORY_EDITOR_API std::filesystem::path GetCurrentPath();
		static GLORY_EDITOR_API void BeginRename(const std::string& name, bool folder);
		static GLORY_EDITOR_API void LoadProject();

		static GLORY_EDITOR_API void OnFileDragAndDrop(const std::filesystem::path& path);

	private:
		virtual void OnOpen() override;
		virtual void OnClose() override;
		virtual void OnGUI() override;

	private:
		static void OnAsyncImport(Resource*);

		void DirectoryBrowser();
		void FileBrowserMenu();
		void DrawPathControls();
		void DrawSearchBar();
		void DrawFileBrowser();
		void RefreshContentBrowser();

		void LoadItems();

	private:
		int m_I;
		static int m_IconSize;
		std::hash<std::string> m_Hasher;
		char m_SearchBuffer[100];

		std::vector<FileBrowserItem*> m_pRootItems;
	};
}