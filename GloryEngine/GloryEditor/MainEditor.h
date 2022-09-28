#pragma once
#include "ProjectPopup.h"
#include "AssetPickerPopup.h"
#include "EditorSettings.h"
#include "EditorAssetLoader.h"
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

	private:
		void CreateDefaultMainMenuBar();
		void SetDarkThemeColors();
		void CreateDefaultObjectMenu();

		void Update();

		void RegisterWindows();
		void RegisterPropertyDrawers();
		void RegisterEditors();

		void Dockspace();
		void ToolbarUI();
		void DrawUserEditor();

	private:
		friend class EditorApplication;
		ProjectPopup* m_pProjectPopup;
		AssetPickerPopup* m_AssetPickerPopup;
		EditorSettings m_Settings;

		static size_t m_SaveSceneIndex;

		static float MENUBAR_SIZE;

		const float TOOLBAR_SIZE = 50;
	};
}
