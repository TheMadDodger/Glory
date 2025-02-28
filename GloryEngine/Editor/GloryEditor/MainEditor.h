#pragma once
#include "ProjectPopup.h"
#include "EditorSettings.h"
#include "Toolbar.h"
#include "PackagePopup.h"
#include "MainWindow.h"

#include <imgui_internal.h>

#define OBJECT_CREATE_MENU(name, component) std::stringstream name##MenuName; \
name##MenuName << STRINGIFY(Create/Entity Object/) << EntitySceneObjectEditor::GetComponentIcon<component>() << "  " << STRINGIFY(name); \
ObjectMenu::AddMenuItem(name##MenuName.str(), Create##name, T_SceneObject | T_Scene | T_Hierarchy);

namespace Glory::Editor
{
	class MainEditor
	{
	public:
		MainEditor();
		virtual ~MainEditor();

		void Initialize();
		void Destroy();
		void PaintEditor();
		void OnFileDragAndDrop(std::vector<std::string_view>& paths);

		static float MENUBAR_SIZE;
		static float WORKTABS_SIZE;

		GLORY_EDITOR_API EditorSettings& Settings();
		static void VersionOutdated(const Glory::Version& latestVersion);

	private:
		void SetupTitleBar();
		void CreateDefaultMainMenuBar();
		void SetDarkThemeColors();
		void CreateDefaultObjectMenu();

		void Update();

		void RegisterWindows();
		void RegisterPropertyDrawers();
		void RegisterEditors();

		void DrawUserEditor();

		void DrawAboutPopup();

	private:
		friend class EditorApplication;
		ProjectPopup* m_pProjectPopup;
		EditorSettings m_Settings;

		PackagePopup m_PackagePopup;

		static size_t m_SaveSceneIndex;

		bool m_OpenAboutPopup = false;

		std::vector<MainWindow*> m_pMainWindows;
	};
}
