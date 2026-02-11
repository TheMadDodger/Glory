#include "EditorApplication.h"
#include "MainEditor.h"
#include "EditorWindow.h"
#include "GameWindow.h"
#include "SceneWindow.h"
#include "InspectorWindow.h"
#include "SceneGraphWindow.h"
#include "FileBrowser.h"
#include "ResourcesWindow.h"
#include "EnvironmentGenerator.h"
#include "EditorConsoleWindow.h"
#include "HistoryWindow.h"
#include "PerformanceMetrics.h"
#include "MenuBar.h"
#include "PopupManager.h"
#include "EditorPreferencesWindow.h"
#include "ProjectSettingsWindow.h"
#include "Window.h"
#include "PopupManager.h"
#include "EditorAssets.h"
#include "ProjectSpace.h"
#include "ThumbnailManager.h"
#include "ImageThumbnailGenerator.h"
#include "TextureThumbnailGenerator.h"
#include "SceneThumbnailGenerator.h"
#include "Editor.h"
#include "ProfilerWindow.h"
#include "EditorSceneManager.h"
#include "AssetReferencePropertyDrawer.h"
#include "ArrayPropertyDrawer.h"
#include "SceneObjectPropertyDrawer.h"
#include "ShapePropertyDrawer.h"
#include "Gizmos.h"
#include "ObjectMenu.h"
#include "ObjectMenuCallbacks.h"
#include "ImGuiHelpers.h"
#include "EnumPropertyDrawer.h"
#include "StructPropertyDrawer.h"
#include "EditorAssetDatabase.h"
#include "SettingsEnums.h"

#include "MaterialEditor.h"
#include "PipelineEditor.h"
#include "GSceneEditor.h"
#include "TextComponentEditor.h"
#include "LightComponentEditor.h"
#include "TextureDataEditor.h"
#include "EntitySceneObjectEditor.h"
#include "DefaultComponentEditor.h"
#include "TransformEditor.h"
#include "MeshRendererEditor.h"

#include "Importer.h"
#include "MaterialImporter.h"
#include "TextureImporter.h"
#include "EntityPrefabImporter.h"
#include "ShaderImporter.h"
#include "CubemapImporter.h"
#include "PipelineImporter.h"
#include "TextImporter.h"

#include "Shortcuts.h"
#include "TitleBar.h"

#include "QuitPopup.h"
#include "VersionPopup.h"
#include "RemovedAssetsPopup.h"

#include "CreateEntityObjectsCallbacks.h"
#include "EditableEntity.h"

#include "Package.h"
#include "About.h"
#include "Dispatcher.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include <Engine.h>
#include <Components.h>
#include <Debug.h>

#define GIZMO_MENU(path, var, value, shortcut) MenuBar::AddMenuItem(path, []() { if(var == value) Gizmos::ToggleMode(); var = value; }, []() { return var == value; }, shortcut)
#define GIZMO_MODE_MENU(path, var, value, shortcut) MenuBar::AddMenuItem(path, []() { var = value; }, []() { return var == value; }, shortcut)

namespace Glory::Editor
{
	static constexpr char* Shortcut_File_NewScene			= "New Scene";
	static constexpr char* Shortcut_File_SaveScene			= "Save Scene";
	static constexpr char* Shortcut_File_LoadScene			= "Load Scene";
	static constexpr char* Shortcut_File_Exit				= "Quit";
	static constexpr char* Shortcut_File_Preferences		= "Preferences";
	static constexpr char* Shortcut_File_SaveProject		= "Save Project";
	static constexpr char* Shortcut_Play_Start				= "Enter Playmode";
	static constexpr char* Shortcut_Play_Stop				= "Stop Playmode";
	static constexpr char* Shortcut_Play_Pauze				= "Toggle Pauze Playmode";
	static constexpr char* Shortcut_Play_NextFrame			= "Playmode Next Frame";
	static constexpr char* Shortcut_About					= "Open About";
	static constexpr char* Shortcut_Window_Scene			= "Open Scene View";
	static constexpr char* Shortcut_Window_Game				= "Open Game View";
	static constexpr char* Shortcut_Window_SceneGraph		= "Open Scene Graph";
	static constexpr char* Shortcut_Window_Inspector		= "Open New Inspector Window";
	static constexpr char* Shortcut_Window_Content			= "Open Content Browser";
	static constexpr char* Shortcut_Window_Console			= "Open Console";
	static constexpr char* Shortcut_Window_Performance		= "Open Performance Metrics";
	static constexpr char* Shortcut_Window_Profiler			= "Open Profiler";
	static constexpr char* Shortcut_Window_ProjectSettings	= "Open Project Settings";
	static constexpr char* Shortcut_Window_Resources		= "Open Resources";
	static constexpr char* Shortcut_Window_Environment		= "Open Environment Generator";
	static constexpr char* Shortcut_View_Perspective		= "Switch To Perspective";
	static constexpr char* Shortcut_View_Orthographic		= "Switch To Orthographic";
	static constexpr char* Shortcut_Edit_Undo				= "Undo";
	static constexpr char* Shortcut_Edit_Redo				= "Redo";
	static constexpr char* Shortcut_Edit_History			= "Edit History";

	static constexpr char* Shortcut_Copy					= "Copy";
	static constexpr char* Shortcut_Paste					= "Paste";
	static constexpr char* Shortcut_Duplicate				= "Duplicate";
	static constexpr char* Shortcut_Delete					= "Delete";
	static constexpr char* Shortcut_Rename					= "Rename";
	static constexpr char* Shortcut_Package					= "Package";

	size_t MainEditor::m_SaveSceneIndex = 0;
	float MainEditor::MENUBAR_SIZE = 0.0f;
	float MainEditor::WORKTABS_SIZE = 65.0f;

	SceneEditingMainWindow MainSceneWindow;

	size_t TabIndex = 0;

	MainEditor::MainEditor()
		: m_pProjectPopup(new ProjectPopup()), m_Settings("./EditorSettings.yaml"),
		m_pMainWindows{ &MainSceneWindow }
	{
	}

	MainEditor::~MainEditor()
	{
		delete m_pProjectPopup;
		m_pProjectPopup = nullptr;

		m_pMainWindows.clear();
	}

	void MainEditor::Initialize()
	{
		SetupTitleBar();
		RegisterWindows();
		RegisterPropertyDrawers();
		RegisterEditors();

		CreateDefaultMainMenuBar();
		CreateDefaultObjectMenu();

		SetDarkThemeColors();

		m_pProjectPopup->Initialize();

		//if (ProjectSpace::GetOpenProject() == nullptr)
		//	m_pProjectPopup->Open();

		ThumbnailManager& thumbnails = EditorApplication::GetInstance()->GetThumbnailManager();
		thumbnails.AddGenerator<ImageThumbnailGenerator>();
		thumbnails.AddGenerator<TextureThumbnailGenerator>();
		thumbnails.AddGenerator<SceneThumbnailGenerator>();

		Gizmos::Initialize();

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		m_Settings.Load(pEngine);

		pEngine->GetDebug().LogInfo("Initialized editor");

		Importer::Register<PipelineImporter>();
		Importer::Register<MaterialImporter>();
		Importer::Register<TextureImporter>();
		Importer::Register<EntityPrefabImporter>();
		Importer::Register<ShaderImporter>();
		Importer::Register<CubemapImporter>();
		Importer::Register<TextImporter>();

		pEngine->GetResourceTypes().RegisterResource<EditableEntity>("");
		pEngine->GetResourceTypes().RegisterResource<ShaderSourceData>("");

		RegisterSettingsEnums(pEngine);

		for (size_t i = 0; i < m_pMainWindows.size(); ++i)
		{
			MainWindow* pWindow = m_pMainWindows[i];
			pWindow->Initialize();
		}
	}

	void MainEditor::Destroy()
	{
		ObjectMenu::Cleanup();

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		m_Settings.Save(pEngine);
		Shortcuts::Clear();

		ProjectSpace::CloseProject();
		PropertyDrawer::Cleanup();

		Gizmos::Cleanup();

		Importer::CleanupAll();
	}

    static void HelpMarker(const char* desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

	void MainEditor::PaintEditor()
	{
		MenuBar::OnGUI();

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
		ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, WORKTABS_SIZE));
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGuiWindowFlags window_flags = 0
			| ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::Begin("Worktabs", NULL, window_flags);
		ImGui::BeginTabBar("Worktabs");
		for (size_t i = 0; i < m_pMainWindows.size(); i++)
		{
			MainWindow* pWindow = m_pMainWindows[i];
			if (ImGui::BeginTabItem(pWindow->Name().data(), NULL, ForceTabIndex == i ? ImGuiTabItemFlags_SetSelected : 0))
			{
				if (TabIndex != i)
				{
					TabIndex = i;
				}
				ImGui::EndTabItem();
			}
		}
		ForceTabIndex = -1;
		ImGui::EndTabBar();

		/* Save off menu bar height for later. */
		MENUBAR_SIZE = ImGui::GetCurrentWindow()->MenuBarHeight();
		ImGui::End();

		m_pMainWindows[TabIndex]->DrawGui(WORKTABS_SIZE - MENUBAR_SIZE);
	}

	void MainEditor::DrawPopups()
	{
		PopupManager::OnGUI();
		m_PackagePopup.Draw();
		ObjectMenu::OnGUI();
		QuitPopup::Draw();
		RemovedAssetsPopup::Draw();
		VersionPopup::Draw();
		DrawAboutPopup();
	}

	void MainEditor::DrawAboutPopup()
	{
		static bool popupOpen = false;
		if (m_OpenAboutPopup)
		{
			popupOpen = true;
			m_OpenAboutPopup = false;
			ImGui::OpenPopup("About");
		}
		if (ImGui::BeginPopupModal("About", &popupOpen, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
		{
			ImGui::Text("Glorious version %s", GloryEditorVersion);
			ImGui::TextUnformatted(AboutText);
			ImGui::EndPopup();
		}
	}

	void MainEditor::OnFileDragAndDrop(std::vector<std::string_view>& paths)
	{
		FileBrowser::OnFileDragAndDrop(paths);
	}

	EditorSettings& MainEditor::Settings()
	{
		return m_Settings;
	}
	
	void MainEditor::VersionOutdated(const Glory::Version& latestVersion)
	{
		std::string latestVersionString;
		latestVersion.GetVersionString(latestVersionString);
		VersionPopup::Open(latestVersionString);
	}

	MainWindow* MainEditor::GetMainWindow(std::type_index type)
	{
		for (size_t i = 0; i < m_pMainWindows.size(); ++i)
		{
			MainWindow* pWindow = m_pMainWindows[i];
			if (pWindow->Type() != type) continue;
			ForceTabIndex = i;
			return pWindow;
		}
		return nullptr;
	}

	MainWindow* MainEditor::FindMainWindow(std::type_index type)
	{
		for (size_t i = 0; i < m_pMainWindows.size(); ++i)
		{
			MainWindow* pWindow = m_pMainWindows[i];
			if (pWindow->Type() != type) continue;
			return pWindow;
		}
		return nullptr;
	}

	void MainEditor::RegisterMainWindow(MainWindow* pWindow)
	{
		pWindow->m_MainWindowIndex = m_pMainWindows.size();
		m_pMainWindows.push_back(pWindow);
		pWindow->Initialize();
	}

	void MainEditor::SetupTitleBar()
	{
		TitleBar::AddSection("Application", "Glorious");
		TitleBar::AddSection("ApplicationSpace", " ");
		TitleBar::AddSection("Version", GloryEditorVersion);
		TitleBar::AddSection("VersionSpace", " - ");

		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		TitleBar::AddSection("Project", pProject ? pProject->Name().c_str() : "No Project open");
		TitleBar::AddSection("ProjectChanges", "");
		TitleBar::AddSection("ProjectSpace", " - ");
		TitleBar::AddSection("Scene", "No Scene open");
		TitleBar::AddSection("SceneChanges", "");
		TitleBar::UpdateTitlebarText();
	}

	void MainEditor::CreateDefaultMainMenuBar()
	{
		EditorApplication* app = EditorApplication::GetInstance();
		MenuBar::AddMenuItem("File/New/Scene", [app]() { app->GetSceneManager().NewScene(); }, NULL, Shortcut_File_NewScene);
		MenuBar::AddMenuItem("File/Save Scene", [app]() { app->GetSceneManager().SaveOpenScenes(); }, NULL, Shortcut_File_SaveScene);

		MenuBar::AddMenuItem("File/Preferences", [this]() { GetWindow<EditorPreferencesWindow>(); }, NULL, Shortcut_File_Preferences);
		MenuBar::AddMenuItem("File/Save Project", [this]() { ProjectSpace::Save(); }, NULL, Shortcut_File_SaveProject);
		MenuBar::AddMenuItem("File/Package", [app]() { StartPackage(app->GetEngine()); }, NULL, Shortcut_Package);

		MenuBar::AddMenuItem("File/About", [&]() { m_OpenAboutPopup = true; }, NULL, Shortcut_About);
		MenuBar::AddMenuItem("File/Exit", [app]() { app->TryToQuit(); }, NULL, Shortcut_File_Exit);

		MenuBar::AddMenuItem("Edit/Undo", Undo::DoUndo, NULL, Shortcut_Edit_Undo);
		MenuBar::AddMenuItem("Edit/Redo", Undo::DoRedo, NULL, Shortcut_Edit_Redo);
		MenuBar::AddMenuItem("Edit/History", [this]() { GetWindow<HistoryWindow>(); }, NULL, Shortcut_Edit_History);

		MenuBar::AddMenuItem("Window/Scene View", [this]() { GetWindow<SceneWindow>(); }, NULL, Shortcut_Window_Scene);
		MenuBar::AddMenuItem("Window/Game View", [this]() { GetWindow<GameWindow>(); }, NULL, Shortcut_Window_Game);
		MenuBar::AddMenuItem("Window/Scene Graph", [this]() { GetWindow<SceneGraphWindow>(); }, NULL, Shortcut_Window_SceneGraph);
		MenuBar::AddMenuItem("Window/Inspector", [this]() { GetWindow<InspectorWindow>(true); }, NULL, Shortcut_Window_Inspector);
		MenuBar::AddMenuItem("Window/Content Browser", [this]() { GetWindow<FileBrowser>(); }, NULL, Shortcut_Window_Content);
		MenuBar::AddMenuItem("Window/Console", [this]() { GetWindow<EditorConsoleWindow>(); }, NULL, Shortcut_Window_Console);
		MenuBar::AddMenuItem("Window/Analysis/Performance Metrics", [this]() { GetWindow<PerformanceMetrics>(); }, NULL, Shortcut_Window_Performance);
		MenuBar::AddMenuItem("Window/Analysis/Profiler", [this]() { GetWindow<ProfilerWindow>(); }, NULL, Shortcut_Window_Profiler);
		MenuBar::AddMenuItem("Window/Project Settings", [this]() { GetWindow<ProjectSettingsWindow>(); }, NULL, Shortcut_Window_ProjectSettings);
		MenuBar::AddMenuItem("Window/Resources", [this]() { GetWindow<ResourcesWindow>(); }, NULL, Shortcut_Window_Resources);
		MenuBar::AddMenuItem("Window/Environment Generator", [this]() { GetWindow<EnvironmentGenerator>(); }, NULL, Shortcut_Window_Environment);

		MenuBar::AddMenuItem("Play/Start", [app]() { app->StartPlay(); }, NULL, Shortcut_Play_Start);
		MenuBar::AddMenuItem("Play/Stop", [app]() { app->StopPlay(); }, NULL, Shortcut_Play_Stop);
		MenuBar::AddMenuItem("Play/Pauze", [app]() { app->TogglePause(); }, NULL, Shortcut_Play_Pauze);
		MenuBar::AddMenuItem("Play/Next Frame", [app]() { app->TickFrame(); }, NULL, Shortcut_Play_NextFrame);

		GIZMO_MENU("Gizmos/Operation/Translate", Gizmos::m_DefaultOperation, ImGuizmo::TRANSLATE, Gizmos::Shortcut_Gizmos_Translate);
		GIZMO_MENU("Gizmos/Operation/Rotate", Gizmos::m_DefaultOperation, ImGuizmo::ROTATE, Gizmos::Shortcut_Gizmos_Rotate);
		GIZMO_MENU("Gizmos/Operation/Scale", Gizmos::m_DefaultOperation, ImGuizmo::SCALE, Gizmos::Shortcut_Gizmos_Scale);
		GIZMO_MENU("Gizmos/Operation/Universal", Gizmos::m_DefaultOperation, ImGuizmo::UNIVERSAL, Gizmos::Shortcut_Gizmos_Universal);

		GIZMO_MODE_MENU("Gizmos/Mode/Local", Gizmos::m_DefaultMode, ImGuizmo::LOCAL, Gizmos::Shortcut_Gizmos_Local);
		GIZMO_MODE_MENU("Gizmos/Mode/World", Gizmos::m_DefaultMode, ImGuizmo::WORLD, Gizmos::Shortcut_Gizmos_World);

		Shortcuts::AddAction(Shortcut_View_Perspective, [&]() {SceneWindow::GetViewEventDispatcher().Dispatch({ false }); });
		Shortcuts::AddAction(Shortcut_View_Orthographic, [&]() {SceneWindow::GetViewEventDispatcher().Dispatch({ true }); });

		Shortcuts::SetShortcut(Shortcut_File_NewScene, ImGuiKey_N, ImGuiMod_Ctrl);
		Shortcuts::SetShortcut(Shortcut_File_SaveScene, ImGuiKey_S, ImGuiMod_Ctrl);
		Shortcuts::SetShortcut(Shortcut_File_LoadScene, ImGuiKey_O, ImGuiMod_Ctrl);
		Shortcuts::SetShortcut(Shortcut_File_Exit, ImGuiKey_Escape, ImGuiMod_Ctrl);
		Shortcuts::SetShortcut(Shortcut_File_Preferences, ImGuiKey_P, ImGuiMod_Ctrl);
		Shortcuts::SetShortcut(Shortcut_File_SaveProject, ImGuiKey_S, ImGuiMod_Ctrl | ImGuiMod_Shift);
		Shortcuts::SetShortcut(Shortcut_About, ImGuiKey_F1, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Play_Start, ImGuiKey_F5, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Play_Stop, ImGuiKey_F6, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Play_Pauze, ImGuiKey_F7, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Play_NextFrame, ImGuiKey_F8, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Scene, ImGuiKey_1, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Game, ImGuiKey_2, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_SceneGraph, ImGuiKey_3, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Inspector, ImGuiKey_4, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Content, ImGuiKey_5, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Console, ImGuiKey_6, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Performance, ImGuiKey_7, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Profiler, ImGuiKey_8, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_ProjectSettings, ImGuiKey_9, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_View_Perspective, ImGuiKey_P, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_View_Orthographic, ImGuiKey_O, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Edit_Undo, ImGuiKey_Z, ImGuiMod_Ctrl);
		Shortcuts::SetShortcut(Shortcut_Edit_Redo, ImGuiKey_Z, ImGuiMod_Ctrl | ImGuiMod_Shift);
		Shortcuts::SetShortcut(Shortcut_Edit_History, ImGuiKey_H, ImGuiMod_Ctrl | ImGuiMod_Shift);
		Shortcuts::SetShortcut(Gizmos::Shortcut_Gizmos_Translate, ImGuiKey_W, ImGuiMod_None);
		Shortcuts::SetShortcut(Gizmos::Shortcut_Gizmos_Rotate, ImGuiKey_R, ImGuiMod_None);
		Shortcuts::SetShortcut(Gizmos::Shortcut_Gizmos_Scale, ImGuiKey_S, ImGuiMod_None);
		Shortcuts::SetShortcut(Gizmos::Shortcut_Gizmos_Universal, ImGuiKey_U, ImGuiMod_None);
		Shortcuts::SetShortcut(Gizmos::Shortcut_Gizmos_Local, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Gizmos::Shortcut_Gizmos_World, ImGuiKey_None, ImGuiMod_None);
	}

	void MainEditor::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);

		// Headers
		colors[ImGuiCol_Header] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

		// Buttons
		colors[ImGuiCol_Button] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

		// Frames BG
		colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.38f, 0.3805f, 0.381f, 1.0f);
		colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.2805f, 0.281f, 1.0f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
	}

	void MainEditor::CreateDefaultObjectMenu()
	{
		ObjectMenu::Initialize();
		ObjectMenu::AddMenuItem("Save Scene", SaveScene, T_Scene);
		ObjectMenu::AddMenuItem("Save Scene As", SaveSceneAs, T_Scene);
		ObjectMenu::AddMenuItem("Set As Active Scene", SetActiveSceneCallback, T_Scene);
		ObjectMenu::AddMenuItem("Remove Scene", RemoveSceneCallback, T_Scene);
		ObjectMenu::AddMenuItem("Reload Scene", ReloadSceneCallback, T_Scene);
		ObjectMenu::AddMenuItemMainWindow("Copy", CopyObjectCallback, T_Resource | T_SceneObject | T_Folder, Shortcut_Copy);
		ObjectMenu::AddMenuItemMainWindow("Paste", PasteObjectCallback, T_SceneObject | T_Resource | T_ContentBrowser | T_Hierarchy | T_Scene | T_Folder, Shortcut_Paste);
		ObjectMenu::AddMenuItemMainWindow("Duplicate", DuplicateObjectCallback, T_SceneObject | T_Resource | T_Folder, Shortcut_Duplicate);
		ObjectMenu::AddMenuItemMainWindow("Delete", DeleteObjectCallback, T_SceneObject | T_Resource | T_Folder, Shortcut_Delete);
		ObjectMenu::AddMenuItem("Create/New Scene", CreateNewSceneCallback, T_Hierarchy);
		ObjectMenu::AddMenuItem("Create/Empty Object", CreateEmptyObjectCallback, T_SceneObject | T_Scene | T_Hierarchy);
		ObjectMenu::AddMenuItem("Create/Texture", CreateNewTextureCallback, T_ContentBrowser | T_Resource);
		ObjectMenu::AddMenuItem("Create/Pipeline", CreateNewPipelineCallback, T_ContentBrowser);
		ObjectMenu::AddMenuItem("Create/Material", CreateNewMaterialCallback, T_ContentBrowser | T_Resource);
		ObjectMenu::AddMenuItem("Create/Folder", CreateNewFolderCallback, T_ContentBrowser | T_Resource);
		ObjectMenu::AddMenuItemMainWindow("Rename", RenameItemCallback, T_Resource | T_Folder, Shortcut_Rename);
		ObjectMenu::AddMenuItem("Reimport", ReimportAssetCallback, T_Resource);

		OBJECT_CREATE_MENU(Mesh, MeshRenderer);
		OBJECT_CREATE_MENU(Model, ModelRenderer);
		OBJECT_CREATE_MENU(Camera, CameraComponent);
		OBJECT_CREATE_MENU(Light, LightComponent);
		OBJECT_CREATE_MENU(Text, TextComponent);

		ObjectMenu::AddMenuItem("Convert to Prefab", &EntitySceneObjectEditor::ConvertToPrefabMenuItem, T_SceneObject);
		ObjectMenu::AddMenuItem("Unpack Prefab", &EntitySceneObjectEditor::UnpackPrefabMenuItem, T_SceneObject);

		Shortcuts::SetShortcut(Shortcut_Copy, ImGuiKey_C, ImGuiModFlags_Ctrl);
		Shortcuts::SetShortcut(Shortcut_Paste, ImGuiKey_V, ImGuiModFlags_Ctrl);
		Shortcuts::SetShortcut(Shortcut_Duplicate, ImGuiKey_D, ImGuiModFlags_Ctrl);
		Shortcuts::SetShortcut(Shortcut_Delete, ImGuiKey_Delete, ImGuiModFlags_None);
		Shortcuts::SetShortcut(Shortcut_Rename, ImGuiKey_F2, ImGuiModFlags_None);
		Shortcuts::SetShortcut(Shortcut_Package, ImGuiKey_P, ImGuiModFlags_Ctrl);

		FileBrowserItem::ObjectDNDEventDispatcher().AddListener([](const FileBrowserItem::ObjectDNDEvent& e) {
			Entity entity = EditorApplication::GetInstance()->GetSceneManager().GetOpenScene(e.Object->SceneID())->GetEntityByEntityID(e.Object->EntityID());
			EntitySceneObjectEditor::ConvertToPrefab(entity, e.Path);
		});
	}

	void MainEditor::Update()
	{
		Shortcuts::Update(TabIndex);
		UpdateWindows();
	}

	void MainEditor::UpdateWindows()
	{
		for (MainWindow* pWindow : m_pMainWindows)
		{
			pWindow->UpdateWindows();
		}
	}

	void MainEditor::RegisterWindows()
	{
		GetWindow<GameWindow>();
		GetWindow<SceneWindow>();
		GetWindow<InspectorWindow>();
		GetWindow<SceneGraphWindow>();
		GetWindow<FileBrowser>();
		GetWindow<EditorConsoleWindow>();
	}

	void MainEditor::RegisterPropertyDrawers()
	{
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<float>>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<int>>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<uint32_t>>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<double>>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<bool>>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<glm::vec2>>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<glm::vec3>>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<glm::vec4>>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<glm::quat>>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<LayerMask>>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<LayerRef>>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<std::string>>();

		PropertyDrawer::RegisterPropertyDrawer<AssetReferencePropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<ArrayPropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<EnumPropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<StructPropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<SceneObjectRef>>();
		PropertyDrawer::RegisterPropertyDrawer<SceneObjectRedirectPropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<ShapePropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<SimplePropertyDrawerTemplate<MeshMaterial>>();
		PropertyDrawer::RegisterPropertyDrawer<BufferPropertyDrawer>();
	}

	void MainEditor::RegisterEditors()
	{
		Editor::RegisterEditor<TextureDataEditor>();
		Editor::RegisterEditor<StaticTextureDataEditor>();
		Editor::RegisterEditor<PipelineEditor>();
		Editor::RegisterEditor<MaterialEditor>();
		Editor::RegisterEditor<StaticMaterialEditor>();

		Editor::RegisterEditor<EntitySceneObjectEditor>();
		Editor::RegisterEditor<DefaultComponentEditor>();
		Editor::RegisterEditor<TransformEditor>();
		Editor::RegisterEditor<MeshRendererEditor>();
		Editor::RegisterEditor<LightComponentEditor>();
		Editor::RegisterEditor<GSceneEditor>();
		Editor::RegisterEditor<TextComponentEditor>();
	}
}
