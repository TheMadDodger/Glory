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
		void DrawPopups();
		void OnFileDragAndDrop(std::vector<std::string_view>& paths);

		static float MENUBAR_SIZE;
		static float WORKTABS_SIZE;

		GLORY_EDITOR_API EditorSettings& Settings();
		static void VersionOutdated(const Glory::Version& latestVersion);

		template<typename T>
		T* GetMainWindow()
		{
			MainWindow* pWindow = GetMainWindow(typeid(T));
			if (!pWindow) return nullptr;
			return static_cast<T*>(pWindow);
		}

		GLORY_EDITOR_API MainWindow* GetMainWindow(std::type_index type);

		template<typename T>
		T* FindMainWindow()
		{
			MainWindow* pWindow = FindMainWindow(typeid(T));
			if (!pWindow) return nullptr;
			return static_cast<T*>(pWindow);
		}

		GLORY_EDITOR_API MainWindow* FindMainWindow(std::type_index type);

		template<typename Sub>
		Sub* GetWindow(bool alwaysOpenNew = false)
		{
			return GetWindow<SceneEditingMainWindow, Sub>(alwaysOpenNew);
		}

		template<typename Main, typename Sub>
		Sub* GetWindow(bool alwaysOpenNew = false)
		{
			Main* pMain = GetMainWindow<Main>();
			return pMain->GetWindow<Sub>(alwaysOpenNew);
		}

		template<typename Sub>
		Sub* FindEditorWindow()
		{
			return FindEditorWindow<SceneEditingMainWindow, Sub>();
		}

		template<typename Main, typename Sub>
		Sub* FindEditorWindow()
		{
			Main* pMain = FindMainWindow<Main>();
			EditorWindow* pWindow = pMain->FindEditorWindow(typeid(Sub));
			if (!pWindow) return nullptr;
			return static_cast<Sub*>(pWindow);
		}

		GLORY_EDITOR_API void RegisterMainWindow(MainWindow* pWindow);

	private:
		void SetupTitleBar();
		void CreateDefaultMainMenuBar();
		void SetDarkThemeColors();
		void CreateDefaultObjectMenu();

		void Update();
		void UpdateWindows();

		void RegisterWindows();
		void RegisterPropertyDrawers();
		void RegisterEditors();

		void DrawAboutPopup();

	private:
		friend class EditorApplication;
		ProjectPopup* m_pProjectPopup;
		EditorSettings m_Settings;

		PackagePopup m_PackagePopup;

		static size_t m_SaveSceneIndex;
		int ForceTabIndex = -1;

		bool m_OpenAboutPopup = false;

		std::vector<MainWindow*> m_pMainWindows;
	};
}
