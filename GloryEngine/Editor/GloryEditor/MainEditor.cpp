#include "EditorApplication.h"
#include "MainEditor.h"
#include "EditorWindow.h"
#include "GameWindow.h"
#include "SceneWindow.h"
#include "InspectorWindow.h"
#include "SceneGraphWindow.h"
#include "FileBrowser.h"
#include "ResourcesWindow.h"
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
#include "Tumbnail.h"
#include "ImageTumbnailGenerator.h"
#include "TextureTumbnailGenerator.h"
#include "SceneTumbnailGenerator.h"
#include "Editor.h"
#include "ProfilerWindow.h"
#include "MaterialEditor.h"
#include "MaterialInstanceEditor.h"
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

#include "Importer.h"
#include "MaterialImporter.h"
#include "MaterialInstanceImporter.h"
#include "TextureImporter.h"

#include "Shortcuts.h"
#include "TitleBar.h"
#include "QuitPopup.h"

#include "VersionPopup.h"

#include "CreateEntityObjectsCallbacks.h"
#include "EditableEntity.h"

#include <imgui.h>
#include <Game.h>
#include <Engine.h>
#include <ImGuizmo.h>
#include <TextureDataEditor.h>
#include <About.h>
#include <Dispatcher.h>
#include <Components.h>
#include <EntitySceneObjectEditor.h>
#include <DefaultComponentEditor.h>
#include <TransformEditor.h>
#include <ScriptedComponentEditor.h>

#define GIZMO_MENU(path, var, value, shortcut) MenuBar::AddMenuItem(path, []() { if(var == value) Gizmos::ToggleMode(); var = value; }, []() { return var == value; }, shortcut)
#define GIZMO_MODE_MENU(path, var, value, shortcut) MenuBar::AddMenuItem(path, []() { var = value; }, []() { return var == value; }, shortcut)

namespace Glory::Editor
{
	static const char* Shortcut_File_NewScene			= "New Scene";
	static const char* Shortcut_File_SaveScene			= "Save Scene";
	static const char* Shortcut_File_LoadScene			= "Load Scene";
	static const char* Shortcut_File_Exit				= "Quit";
	static const char* Shortcut_File_Preferences		= "Preferences";
	static const char* Shortcut_File_SaveProject		= "Save Project";
	static const char* Shortcut_Play_Start				= "Enter Playmode";
	static const char* Shortcut_Play_Stop				= "Stop Playmode";
	static const char* Shortcut_Play_Pauze				= "Toggle Pauze Playmode";
	static const char* Shortcut_Play_NextFrame			= "Playmode Next Frame";
	static const char* Shortcut_Window_Scene			= "Open Scene View";
	static const char* Shortcut_Window_Game				= "Open Game View";
	static const char* Shortcut_Window_SceneGraph		= "Open Scene Graph";
	static const char* Shortcut_Window_Inspector		= "Open New Inspector Window";
	static const char* Shortcut_Window_Content			= "Open Content Browser";
	static const char* Shortcut_Window_Console			= "Open Console";
	static const char* Shortcut_Window_Performance		= "Open Performance Metrics";
	static const char* Shortcut_Window_Profiler			= "Open Profiler";
	static const char* Shortcut_Window_ProjectSettings	= "Open Project Settings";
	static const char* Shortcut_Window_Resources		= "Open Resources";
	static const char* Shortcut_View_Perspective		= "Switch To Perspective";
	static const char* Shortcut_View_Orthographic		= "Switch To Orthographic";
	static const char* Shortcut_Edit_Undo				= "Undo";
	static const char* Shortcut_Edit_Redo				= "Redo";
	static const char* Shortcut_Edit_History			= "Edit History";

	static const char* Shortcut_Copy					= "Copy";
	static const char* Shortcut_Paste					= "Paste";
	static const char* Shortcut_Duplicate				= "Duplicate";
	static const char* Shortcut_Delete					= "Delete";
	static const char* Shortcut_Rename					= "Rename";

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

		Tumbnail::AddGenerator<ImageTumbnailGenerator>();
		Tumbnail::AddGenerator<TextureTumbnailGenerator>();
		Tumbnail::AddGenerator<SceneTumbnailGenerator>();

		Gizmos::Initialize();

		m_Settings.Load(Game::GetGame().GetEngine());

		Debug::LogInfo("Initialized editor");

		Importer::Register<MaterialImporter>();
		Importer::Register<MaterialInstanceImporter>();
		Importer::Register<TextureImporter>();

		ResourceType::RegisterResource<EditableEntity>("");
	}

	void MainEditor::Destroy()
	{
		ObjectMenu::Cleanup();

		m_Settings.Save(Game::GetGame().GetEngine());
		Shortcuts::Clear();

		ProjectSpace::CloseProject();
		EditorWindow::Cleanup();
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
		QuitPopup::Draw();
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

	void MainEditor::OnFileDragAndDrop(std::string_view path)
	{
		FileBrowser::OnFileDragAndDrop(path);
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
		MenuBar::AddMenuItem("File/New/Scene", []() { EditorSceneManager::NewScene(false); }, NULL, Shortcut_File_NewScene);
		MenuBar::AddMenuItem("File/Save Scene", EditorSceneManager::SaveOpenScenes, NULL, Shortcut_File_SaveScene);
		//MenuBar::AddMenuItem("File/Load Scene", []()
		//{
		//	//YAML::Node node = YAML::LoadFile("test.gscene");
		//	//Serializer::DeserializeObject(node);
		//}, NULL, Shortcut_File_LoadScene);

		MenuBar::AddMenuItem("File/Preferences", []() { EditorWindow::GetWindow<EditorPreferencesWindow>(); }, NULL, Shortcut_File_Preferences);
		MenuBar::AddMenuItem("File/Save Project", []() { ProjectSpace::Save(); }, NULL, Shortcut_File_SaveProject);

		MenuBar::AddMenuItem("File/About", [&]() { m_OpenAboutPopup = true; }, NULL);
		MenuBar::AddMenuItem("File/Exit", EditorApplication::TryToQuit, NULL, Shortcut_File_Exit);

		MenuBar::AddMenuItem("Edit/Undo", Undo::DoUndo, NULL, Shortcut_Edit_Undo);
		MenuBar::AddMenuItem("Edit/Redo", Undo::DoRedo, NULL, Shortcut_Edit_Redo);
		MenuBar::AddMenuItem("Edit/History", []() { EditorWindow::GetWindow<HistoryWindow>(); }, NULL, Shortcut_Edit_History);

		MenuBar::AddMenuItem("Window/Scene View", []() { EditorWindow::GetWindow<SceneWindow>(); }, NULL, Shortcut_Window_Scene);
		MenuBar::AddMenuItem("Window/Game View", []() { EditorWindow::GetWindow<GameWindow>(); }, NULL, Shortcut_Window_Game);
		MenuBar::AddMenuItem("Window/Scene Graph", []() { EditorWindow::GetWindow<SceneGraphWindow>(); }, NULL, Shortcut_Window_SceneGraph);
		MenuBar::AddMenuItem("Window/Inspector", []() { EditorWindow::GetWindow<InspectorWindow>(true); }, NULL, Shortcut_Window_Inspector);
		MenuBar::AddMenuItem("Window/Content Browser", []() { EditorWindow::GetWindow<FileBrowser>(); }, NULL, Shortcut_Window_Content);
		MenuBar::AddMenuItem("Window/Console", []() { EditorWindow::GetWindow<EditorConsoleWindow>(); }, NULL, Shortcut_Window_Console);
		MenuBar::AddMenuItem("Window/Analysis/Performance Metrics", []() { EditorWindow::GetWindow<PerformanceMetrics>(); }, NULL, Shortcut_Window_Performance);
		MenuBar::AddMenuItem("Window/Analysis/Profiler", []() { EditorWindow::GetWindow<ProfilerWindow>(); }, NULL, Shortcut_Window_Profiler);
		MenuBar::AddMenuItem("Window/Project Settings", []() { EditorWindow::GetWindow<ProjectSettingsWindow>(); }, NULL, Shortcut_Window_ProjectSettings);
		MenuBar::AddMenuItem("Window/Resources", []() { EditorWindow::GetWindow<ResourcesWindow>(); }, NULL, Shortcut_Window_Resources);

		MenuBar::AddMenuItem("Play/Start", EditorApplication::StartPlay, NULL, Shortcut_Play_Start);
		MenuBar::AddMenuItem("Play/Stop", EditorApplication::StopPlay, NULL, Shortcut_Play_Stop);
		MenuBar::AddMenuItem("Play/Pauze", EditorApplication::TogglePause, NULL, Shortcut_Play_Pauze);
		MenuBar::AddMenuItem("Play/Next Frame", EditorApplication::TickFrame, NULL, Shortcut_Play_NextFrame);

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
		Shortcuts::SetShortcut(Shortcut_File_Preferences, ImGuiKey_F2, ImGuiMod_None);
		Shortcuts::SetShortcut(Shortcut_File_SaveProject, ImGuiKey_S, ImGuiMod_Ctrl | ImGuiMod_Shift);
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
		ObjectMenu::AddMenuItem("Copy", CopyObjectCallback, T_Resource | T_SceneObject | T_Folder, Shortcut_Copy);
		ObjectMenu::AddMenuItem("Paste", PasteObjectCallback, T_SceneObject | T_Resource | T_ContentBrowser | T_Hierarchy | T_Scene | T_Folder, Shortcut_Paste);
		ObjectMenu::AddMenuItem("Duplicate", DuplicateObjectCallback, T_SceneObject | T_Resource | T_Folder, Shortcut_Duplicate);
		ObjectMenu::AddMenuItem("Delete", DeleteObjectCallback, T_SceneObject | T_Resource | T_Folder, Shortcut_Delete);
		ObjectMenu::AddMenuItem("Create/New Scene", CreateNewSceneCallback, T_Hierarchy);
		ObjectMenu::AddMenuItem("Create/Empty Object", CreateEmptyObjectCallback, T_SceneObject | T_Scene | T_Hierarchy);
		ObjectMenu::AddMenuItem("Create/Texture", CreateNewTextureCallback, T_ContentBrowser | T_Resource);
		ObjectMenu::AddMenuItem("Create/Material", CreateNewMaterialCallback, T_ContentBrowser | T_Resource);
		ObjectMenu::AddMenuItem("Create/Material Instance", CreateNewMaterialInstanceCallback, T_ContentBrowser | T_Resource);
		ObjectMenu::AddMenuItem("Create/Folder", CreateNewFolderCallback, T_ContentBrowser | T_Resource);
		ObjectMenu::AddMenuItem("Rename", RenameItemCallback, T_Resource | T_Folder, Shortcut_Rename);
		ObjectMenu::AddMenuItem("Reimport", ReimportAssetCallback, T_Resource);

		OBJECT_CREATE_MENU(Mesh, MeshRenderer);
		OBJECT_CREATE_MENU(Model, ModelRenderer);
		OBJECT_CREATE_MENU(Camera, CameraComponent);
		OBJECT_CREATE_MENU(Light, LightComponent);
		OBJECT_CREATE_MENU(Scripted, ScriptedComponent);

		ObjectMenu::AddMenuItem("Convert to Prefab", &EntitySceneObjectEditor::ConvertToPrefabMenuItem, T_SceneObject);
		ObjectMenu::AddMenuItem("Unpack Prefab", &EntitySceneObjectEditor::UnpackPrefabMenuItem, T_SceneObject);

		Shortcuts::SetShortcut(Shortcut_Copy, ImGuiKey_C, ImGuiModFlags_Ctrl);
		Shortcuts::SetShortcut(Shortcut_Paste, ImGuiKey_V, ImGuiModFlags_Ctrl);
		Shortcuts::SetShortcut(Shortcut_Duplicate, ImGuiKey_D, ImGuiModFlags_Ctrl);
		Shortcuts::SetShortcut(Shortcut_Delete, ImGuiKey_Delete, ImGuiModFlags_None);
		Shortcuts::SetShortcut(Shortcut_Rename, ImGuiKey_R, ImGuiModFlags_Ctrl);

		FileBrowserItem::ObjectDNDEventDispatcher().AddListener([](const FileBrowserItem::ObjectDNDEvent& e) {
			Entity entity = EditorSceneManager::GetOpenScene(e.Object->SceneID())->GetEntityByEntityID(e.Object->EntityID());
			EntitySceneObjectEditor::ConvertToPrefab(entity, e.Path);
		});
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
		EditorWindow::GetWindow<FileBrowser>();
		EditorWindow::GetWindow<EditorConsoleWindow>();
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

		PropertyDrawer::RegisterPropertyDrawer<AssetReferencePropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<ArrayPropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<EnumPropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<StructPropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<PropertyDrawerTemplate<SceneObjectRef>>();
		PropertyDrawer::RegisterPropertyDrawer<SceneObjectRedirectPropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<ShapePropertyDrawer>();
		PropertyDrawer::RegisterPropertyDrawer<SimplePropertyDrawerTemplate<MeshMaterial>>();
	}

	void MainEditor::RegisterEditors()
	{
		Editor::RegisterEditor<TextureDataEditor>();
		Editor::RegisterEditor<MaterialEditor>();
		Editor::RegisterEditor<MaterialInstanceEditor>();

		Editor::RegisterEditor<EntitySceneObjectEditor>();
		Editor::RegisterEditor<DefaultComponentEditor>();
		Editor::RegisterEditor<TransformEditor>();
		Editor::RegisterEditor<ScriptedComponentEditor>();
	}
}
