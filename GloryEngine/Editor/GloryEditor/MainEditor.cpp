#include <imgui.h>
#include <Game.h>
#include <Engine.h>
#include <ImGuizmo.h>
#include <Serializer.h>
#include <AssetDatabase.h>

#include "EditorApplication.h"
#include "MainEditor.h"
#include "EditorWindow.h"
#include "GameWindow.h"
#include "SceneWindow.h"
#include "InspectorWindow.h"
#include "SceneGraphWindow.h"
#include "ContentBrowser.h"
#include "EditorConsoleWindow.h"
#include "HistoryWindow.h"
#include "PerformanceMetrics.h"
#include "MenuBar.h"
#include "PopupManager.h"
#include "EditorPreferencesWindow.h"
#include "Window.h"
#include "PopupManager.h"
#include "EditorAssets.h"
#include "ProjectSpace.h"
#include "Tumbnail.h"
#include "TextureTumbnailGenerator.h"
#include "SceneTumbnailGenerator.h"
#include "Editor.h"
#include "ProfilerWindow.h"
#include "MaterialEditor.h"
#include "StandardPropertyDrawers.h"
#include "MaterialInstanceEditor.h"
#include "EditorSceneManager.h"
#include "AssetReferencePropertyDrawer.h"
#include "ArrayPropertyDrawer.h"
#include "Gizmos.h"
#include "ObjectMenu.h"
#include "ObjectMenuCallbacks.h"
#include "FileDialog.h"
#include "ImGuiHelpers.h"
#include "EnumPropertyDrawer.h"
#include "StructPropertyDrawer.h"

#include "Shortcuts.h"

#define GIZMO_MENU(path, var, value) MenuBar::AddMenuItem(path, []() { var = value; }, []() { return var == value; })

namespace Glory::Editor
{
	static const char* Shortcut_File_NewScene = "Shortcut_File_NewScene";
	static const char* Shortcut_File_SaveScene = "Shortcut_File_SaveScene";
	static const char* Shortcut_File_LoadScene = "Shortcut_File_LoadScene";
	static const char* Shortcut_File_Exit = "Shortcut_File_Exit";
	static const char* Shortcut_File_Preferences = "Shortcut_File_Preferences";
	static const char* Shortcut_File_SaveProject = "Shortcut_File_SaveProject";
	static const char* Shortcut_Play_Start = "Shortcut_Play_Start";
	static const char* Shortcut_Play_Stop = "Shortcut_Play_Stop";
	static const char* Shortcut_Play_Pauze = "Shortcut_Play_Pauze";
	static const char* Shortcut_Play_NextFrame = "Shortcut_Play_NextFrame";
	static const char* Shortcut_Window_Scene = "Shortcut_Window_Scene";
	static const char* Shortcut_Window_Game = "Shortcut_Window_Game";
	static const char* Shortcut_Window_SceneGraph = "Shortcut_Window_SceneGraph";
	static const char* Shortcut_Window_Inspector = "Shortcut_Window_Inspector";
	static const char* Shortcut_Window_Content = "Shortcut_Window_Content";
	static const char* Shortcut_Window_Console = "Shortcut_Window_Console";
	static const char* Shortcut_Window_Performance = "Shortcut_Window_Performance";
	static const char* Shortcut_Window_Profiler = "Shortcut_Window_Profiler";
	static const char* Shortcut_View_Perspective = "Shortcut_View_Perspective";
	static const char* Shortcut_View_Orthographic = "Shortcut_View_Orthographic";
	static const char* Shortcut_Edit_Undo = "Shortcut_Edit_Undo";
	static const char* Shortcut_Edit_Redo = "Shortcut_Edit_Redo";
	static const char* Shortcut_Edit_History = "Shortcut_Edit_History";
	static const char* Shortcut_Gizmos_Translate = "Shortcut_Gizmos_Translate";
	static const char* Shortcut_Gizmos_Rotate = "Shortcut_Gizmos_Rotate";
	static const char* Shortcut_Gizmos_Scale = "Shortcut_Gizmos_Scale";
	static const char* Shortcut_Gizmos_Universal = "Shortcut_Gizmos_Universal";
	static const char* Shortcut_Gizmos_Local = "Shortcut_Gizmos_Local";
	static const char* Shortcut_Gizmos_World = "Shortcut_Gizmos_World";

	size_t MainEditor::m_SaveSceneIndex = 0;
	float MainEditor::MENUBAR_SIZE = 0.0f;
	const float MainEditor::TOOLBAR_SIZE = 50.0f;

	MainEditor::MainEditor()
		: m_pProjectPopup(new ProjectPopup()), m_pToolbar(new Toolbar(TOOLBAR_SIZE)), m_Settings("./EditorSettings.yaml")
	{
	}

	MainEditor::~MainEditor()
	{
		delete m_pProjectPopup;
		m_pProjectPopup = nullptr;

		delete m_pToolbar;
		m_pToolbar = nullptr;
	}

	void MainEditor::Initialize()
	{
		m_Settings.Load(Game::GetGame().GetEngine());

		RegisterWindows();
		RegisterPropertyDrawers();
		RegisterEditors();

		CreateDefaultMainMenuBar();
		CreateDefaultObjectMenu();

		SetDarkThemeColors();

		m_pProjectPopup->Initialize();

		//if (ProjectSpace::GetOpenProject() == nullptr)
		//	m_pProjectPopup->Open();

		Tumbnail::AddGenerator<TextureTumbnailGenerator>();
		Tumbnail::AddGenerator<SceneTumbnailGenerator>();

		FileDialog::Initialize();

		Gizmos::Initialize();
	}

	void MainEditor::Destroy()
	{
		EditorAssetLoader::Stop();

		m_Settings.Save(Game::GetGame().GetEngine());
		Shortcuts::Clear();

		ProjectSpace::CloseProject();
		EditorWindow::Cleanup();
		PropertyDrawer::Cleanup();

		Gizmos::Cleanup();
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
		Dockspace();
		m_pToolbar->Paint();
		DrawUserEditor();
	}

	void MainEditor::Dockspace()
	{
		const float toolbarSize = 50;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos + ImVec2(0, toolbarSize));
		ImGui::SetNextWindowSize(viewport->Size - ImVec2(0, toolbarSize));
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGuiWindowFlags window_flags = 0
			| ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::Begin("Master DockSpace", NULL, window_flags);
		ImGuiID dockMain = ImGui::GetID("MyDockspace");

		// Save off menu bar height for later.
		MENUBAR_SIZE = ImGui::GetCurrentWindow()->MenuBarHeight();

		ImGui::DockSpace(dockMain);
		ImGui::End();
		ImGui::PopStyleVar(3);
	}

	void MainEditor::DrawUserEditor()
	{
		EditorWindow::RenderWindows();
		PopupManager::OnGUI();
		ObjectMenu::OnGUI();
		m_pProjectPopup->OnGui();
		FileDialog::Update();
	}

	void MainEditor::CreateDefaultMainMenuBar()
	{
		MenuBar::AddMenuItem("File/New/Scene", []() { EditorSceneManager::NewScene(false); }, NULL, Shortcut_File_NewScene);
		MenuBar::AddMenuItem("File/Save Scene", EditorSceneManager::SaveOpenScenes, NULL, Shortcut_File_SaveScene);
		MenuBar::AddMenuItem("File/Load Scene", []()
		{
			//YAML::Node node = YAML::LoadFile("test.gscene");
			//Serializer::DeserializeObject(node);
		}, NULL, Shortcut_File_LoadScene);

		MenuBar::AddMenuItem("File/Preferences", []() { EditorWindow::GetWindow<EditorPreferencesWindow>(); }, NULL, Shortcut_File_Preferences);
		MenuBar::AddMenuItem("File/Save Project", []() { AssetDatabase::Save(); ProjectSpace::Save(); }, NULL, Shortcut_File_SaveProject);
		//MenuBar::AddMenuItem("File/Create/Empty Object", []()
		//{
		//	GScene* pActiveScene = Game::GetGame().GetEngine()->GetScenesModule()->GetActiveScene();
		//	if (!pActiveScene) return;
		//	pActiveScene->CreateEmptyObject();
		//});

		MenuBar::AddMenuItem("Play/Start", EditorApplication::StartPlay, NULL, Shortcut_Play_Start);
		MenuBar::AddMenuItem("Play/Stop", EditorApplication::StopPlay, NULL, Shortcut_Play_Stop);
		MenuBar::AddMenuItem("Play/Pauze", EditorApplication::TogglePause, NULL, Shortcut_Play_Pauze);
		MenuBar::AddMenuItem("Play/Next Frame", EditorApplication::TickFrame, NULL, Shortcut_Play_NextFrame);

		MenuBar::AddMenuItem("File/Exit", [&]() {
			std::vector<std::string> buttons = { "Cancel", "Exit" };
			std::vector<std::function<void()>> buttonFuncs = { [&]() { PopupManager::CloseCurrentPopup(); }, [&]() {/*m_IsRunning = false;*/ } };
			PopupManager::OpenPopup("Exit", "Are you sure you want to exit? All unsaved changes will be lost!",
				buttons, buttonFuncs); }, NULL, Shortcut_File_Exit);

		MenuBar::AddMenuItem("Window/Scene View", []() { EditorWindow::GetWindow<SceneWindow>(); }, NULL, Shortcut_Window_Scene);
		MenuBar::AddMenuItem("Window/Game View", []() { EditorWindow::GetWindow<GameWindow>(); }, NULL, Shortcut_Window_Game);
		MenuBar::AddMenuItem("Window/Scene Graph", []() { EditorWindow::GetWindow<SceneGraphWindow>(); }, NULL, Shortcut_Window_SceneGraph);
		MenuBar::AddMenuItem("Window/Inspector", []() { EditorWindow::GetWindow<InspectorWindow>(true); }, NULL, Shortcut_Window_Inspector);
		MenuBar::AddMenuItem("Window/Content Browser", []() { EditorWindow::GetWindow<ContentBrowser>(); }, NULL, Shortcut_Window_Content);
		MenuBar::AddMenuItem("Window/Console", []() { EditorWindow::GetWindow<EditorConsoleWindow>(); }, NULL, Shortcut_Window_Console);
		MenuBar::AddMenuItem("Window/Analysis/Performance Metrics", []() { EditorWindow::GetWindow<PerformanceMetrics>(); }, NULL, Shortcut_Window_Performance);
		MenuBar::AddMenuItem("Window/Analysis/Profiler", []() { EditorWindow::GetWindow<ProfilerWindow>(); }, NULL, Shortcut_Window_Profiler);

		MenuBar::AddMenuItem("View/Perspective", []() { SceneWindow::EnableOrthographicView(false); }, []() { return !SceneWindow::IsOrthographicEnabled(); }, Shortcut_View_Perspective);
		MenuBar::AddMenuItem("View/Orthographic", []() { SceneWindow::EnableOrthographicView(true); }, []() { return SceneWindow::IsOrthographicEnabled(); }, Shortcut_View_Orthographic);

		MenuBar::AddMenuItem("Edit/Undo", Undo::DoUndo, NULL, Shortcut_Edit_Undo);
		MenuBar::AddMenuItem("Edit/Redo", Undo::DoRedo, NULL, Shortcut_Edit_Redo);
		MenuBar::AddMenuItem("Edit/History", []() { EditorWindow::GetWindow<HistoryWindow>(); }, NULL, Shortcut_Edit_History);

		GIZMO_MENU("Gizmos/Operation/Translate", Gizmos::m_DefaultOperation, ImGuizmo::TRANSLATE);
		GIZMO_MENU("Gizmos/Operation/Rotate", Gizmos::m_DefaultOperation, ImGuizmo::ROTATE);
		GIZMO_MENU("Gizmos/Operation/Scale", Gizmos::m_DefaultOperation, ImGuizmo::SCALE);
		GIZMO_MENU("Gizmos/Operation/Universal", Gizmos::m_DefaultOperation, ImGuizmo::UNIVERSAL);

		GIZMO_MENU("Gizmos/Mode/Local", Gizmos::m_DefaultMode, ImGuizmo::LOCAL);
		GIZMO_MENU("Gizmos/Mode/World", Gizmos::m_DefaultMode, ImGuizmo::WORLD);

		Shortcuts::SetShortcut(Shortcut_File_NewScene, ImGuiKey_N, ImGuiMod_Ctrl);
		Shortcuts::SetShortcut(Shortcut_File_SaveScene, ImGuiKey_S, ImGuiMod_Ctrl);
		Shortcuts::SetShortcut(Shortcut_File_LoadScene, ImGuiKey_O, ImGuiMod_Ctrl);
		Shortcuts::SetShortcut(Shortcut_File_Exit, ImGuiKey_Escape, ImGuiMod_Ctrl);
		Shortcuts::SetShortcut(Shortcut_File_Preferences, ImGuiKey_F2, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_File_SaveProject, ImGuiKey_S, ImGuiMod_Ctrl | ImGuiMod_Shift);
		Shortcuts::SetShortcut(Shortcut_Play_Start, ImGuiKey_F5, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Play_Stop, ImGuiKey_F6, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Play_Pauze, ImGuiKey_F7, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Play_NextFrame, ImGuiKey_F8, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Scene, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Game, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_SceneGraph, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Inspector, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Content, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Console, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Performance, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Window_Profiler, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_View_Perspective, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_View_Orthographic, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Edit_Undo, ImGuiKey_Z, ImGuiMod_Ctrl);
		Shortcuts::SetShortcut(Shortcut_Edit_Redo, ImGuiKey_Z, ImGuiMod_Ctrl | ImGuiMod_Shift);
		Shortcuts::SetShortcut(Shortcut_Edit_History, ImGuiKey_H, ImGuiMod_Ctrl | ImGuiMod_Shift);
		Shortcuts::SetShortcut(Shortcut_Gizmos_Translate, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Gizmos_Rotate, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Gizmos_Scale, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Gizmos_Universal, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Gizmos_Local, ImGuiKey_None, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_Gizmos_World, ImGuiKey_None, ImGuiMod_None);
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
		ObjectMenu::AddMenuItem("Save Scene", SaveScene, T_Scene);
		ObjectMenu::AddMenuItem("Save Scene As", SaveSceneAs, T_Scene);
		ObjectMenu::AddMenuItem("Set As Active Scene", SetActiveSceneCallback, T_Scene);
		ObjectMenu::AddMenuItem("Remove Scene", RemoveSceneCallback, T_Scene);
		ObjectMenu::AddMenuItem("Reload Scene", ReloadSceneCallback, T_Scene);
		ObjectMenu::AddMenuItem("Copy", CopyObjectCallback, T_AnyResource | T_SceneObject | T_Scene);
		ObjectMenu::AddMenuItem("Paste", PasteObjectCallback, T_SceneObject | T_Resource | T_ContentBrowser | T_Hierarchy | T_Scene);
		ObjectMenu::AddMenuItem("Duplicate", DuplicateObjectCallback, T_SceneObject | T_AnyResource);
		ObjectMenu::AddMenuItem("Delete", DeleteObjectCallback, T_SceneObject | T_Resource | T_Folder);
		ObjectMenu::AddMenuItem("Create/Empty Object", CreateEmptyObjectCallback, T_SceneObject | T_Scene | T_Hierarchy);
		ObjectMenu::AddMenuItem("Create/New Scene", CreateNewSceneCallback, T_Hierarchy);
		ObjectMenu::AddMenuItem("Create/Material", CreateNewMaterialCallback, T_ContentBrowser | T_Resource);
		ObjectMenu::AddMenuItem("Create/Material Instance", CreateNewMaterialInstanceCallback, T_ContentBrowser | T_Resource);
		ObjectMenu::AddMenuItem("Create/Folder", CreateNewFolderCallback, T_ContentBrowser | T_Resource);
		ObjectMenu::AddMenuItem("Rename", RenameItemCallback, T_Resource | T_Folder);
	}

	void MainEditor::Update()
	{
		Shortcuts::Update();
		EditorWindow::UpdateWindows();
	}

	void MainEditor::RegisterWindows()
	{
		EditorWindow::GetWindow<GameWindow>();
		EditorWindow::GetWindow<SceneWindow>();
		EditorWindow::GetWindow<InspectorWindow>();
		EditorWindow::GetWindow<SceneGraphWindow>();
		EditorWindow::GetWindow<ContentBrowser>();
		EditorWindow::GetWindow<EditorConsoleWindow>();
	}

	void MainEditor::RegisterPropertyDrawers()
	{
		PropertyDrawer::RegisterPropertyDrawer<FloatDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<IntDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<BoolDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<DoubleDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<Vector2Drawer>();
		PropertyDrawer::RegisterPropertyDrawer<Vector3Drawer>();
		PropertyDrawer::RegisterPropertyDrawer<Vector4Drawer>();
		PropertyDrawer::RegisterPropertyDrawer<QuatDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<LayerMaskDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<LayerRefDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<AssetReferencePropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<ArrayPropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<EnumPropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<StructPropertyDrawer>();
	}

	void MainEditor::RegisterEditors()
	{
		Editor::RegisterEditor<MaterialEditor>();
		Editor::RegisterEditor<MaterialInstanceEditor>();
	}
}
