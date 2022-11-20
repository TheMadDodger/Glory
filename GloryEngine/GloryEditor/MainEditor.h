#pragma once
#include "ProjectPopup.h"
#include "EditorSettings.h"
#include "EditorAssetLoader.h"
#include "Toolbar.h"
#include <imgui_internal.h>

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

		static float MENUBAR_SIZE;

	private:
		void CreateDefaultMainMenuBar();
		void SetDarkThemeColors();
		void CreateDefaultObjectMenu();

		void Update();

		void RegisterWindows();
		void RegisterPropertyDrawers();
		void RegisterEditors();

		void Dockspace();
		void DrawUserEditor();

	private:
		friend class EditorApplication;
		ProjectPopup* m_pProjectPopup;
		Toolbar* m_pToolbar;
		EditorSettings m_Settings;

		static size_t m_SaveSceneIndex;

		static const float TOOLBAR_SIZE;
	};
}
