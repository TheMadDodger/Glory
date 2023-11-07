#pragma once
#include "ProjectPopup.h"
#include "EditorSettings.h"
#include "Toolbar.h"
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
		void OnFileDragAndDrop(std::string_view path);

		static float MENUBAR_SIZE;

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

		void Dockspace();
		void DrawUserEditor();

		void DrawAboutPopup();

	private:
		friend class EditorApplication;
		ProjectPopup* m_pProjectPopup;
		Toolbar* m_pToolbar;
		EditorSettings m_Settings;

		static size_t m_SaveSceneIndex;

		static const float TOOLBAR_SIZE;
		bool m_OpenAboutPopup = false;
	};
}
