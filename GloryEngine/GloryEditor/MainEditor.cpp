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

#define GIZMO_MENU(path, var, value) MenuBar::AddMenuItem(path, []() { var = value; }, []() { return var == value; })

namespace Glory::Editor
{
	size_t MainEditor::m_SaveSceneIndex = 0;
	float MainEditor::MENUBAR_SIZE = 0.0f;
	const float MainEditor::TOOLBAR_SIZE = 50.0f;

	MainEditor::MainEditor()
		: m_pProjectPopup(new ProjectPopup()), m_AssetPickerPopup(new AssetPickerPopup()), m_pToolbar(new Toolbar(TOOLBAR_SIZE)), m_Settings("./EditorSettings.yaml")
	{
	}

	MainEditor::~MainEditor()
	{
		delete m_pProjectPopup;
		m_pProjectPopup = nullptr;

		delete m_AssetPickerPopup;
		m_AssetPickerPopup = nullptr;

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
		m_AssetPickerPopup->OnGUI();
		FileDialog::Update();
	}

	void MainEditor::CreateDefaultMainMenuBar()
	{
		MenuBar::AddMenuItem("File/New/Scene", []() { EditorSceneManager::NewScene(false); });
		MenuBar::AddMenuItem("File/Save Scene", EditorSceneManager::SaveOpenScenes);
		MenuBar::AddMenuItem("File/Load Scene", []()
		{
			//YAML::Node node = YAML::LoadFile("test.gscene");
			//Serializer::DeserializeObject(node);
		});

		MenuBar::AddMenuItem("File/Preferences", []() { EditorWindow::GetWindow<EditorPreferencesWindow>(); });
		MenuBar::AddMenuItem("File/Save Project", []() { AssetDatabase::Save(); ProjectSpace::Save(); });
		MenuBar::AddMenuItem("File/Create/Empty Object", []()
		{
			GScene* pActiveScene = Game::GetGame().GetEngine()->GetScenesModule()->GetActiveScene();
			if (!pActiveScene) return;
			pActiveScene->CreateEmptyObject();
		});

		MenuBar::AddMenuItem("Play/Start", EditorApplication::StartPlay);
		MenuBar::AddMenuItem("Play/Stop", EditorApplication::StopPlay);
		MenuBar::AddMenuItem("Play/Pauze", EditorApplication::TogglePause);
		MenuBar::AddMenuItem("Play/Next Frame", EditorApplication::TickFrame);

		MenuBar::AddMenuItem("File/Exit", [&]() {
			std::vector<std::string> buttons = { "Cancel", "Exit" };
			std::vector<std::function<void()>> buttonFuncs = { [&]() { PopupManager::CloseCurrentPopup(); }, [&]() {/*m_IsRunning = false;*/ } };
			PopupManager::OpenPopup("Exit", "Are you sure you want to exit? All unsaved changes will be lost!",
				buttons, buttonFuncs); });

		MenuBar::AddMenuItem("Window/Scene View", []() { EditorWindow::GetWindow<SceneWindow>(); });
		MenuBar::AddMenuItem("Window/Game View", []() { EditorWindow::GetWindow<GameWindow>(); });
		MenuBar::AddMenuItem("Window/Scene Graph", []() { EditorWindow::GetWindow<SceneGraphWindow>(); });
		MenuBar::AddMenuItem("Window/Inspector", []() { EditorWindow::GetWindow<InspectorWindow>(true); });
		MenuBar::AddMenuItem("Window/Content Browser", []() { EditorWindow::GetWindow<ContentBrowser>(); });
		MenuBar::AddMenuItem("Window/Console", []() { EditorWindow::GetWindow<EditorConsoleWindow>(); });
		MenuBar::AddMenuItem("Window/Analysis/Performance Metrics", []() { EditorWindow::GetWindow<PerformanceMetrics>(); });
		MenuBar::AddMenuItem("Window/Analysis/Profiler", []() { EditorWindow::GetWindow<ProfilerWindow>(); });

		MenuBar::AddMenuItem("View/Perspective", []() { SceneWindow::EnableOrthographicView(false); }, []() { return !SceneWindow::IsOrthographicEnabled(); });
		MenuBar::AddMenuItem("View/Orthographic", []() { SceneWindow::EnableOrthographicView(true); }, []() { return SceneWindow::IsOrthographicEnabled(); });

		MenuBar::AddMenuItem("Edit/Undo", Undo::DoUndo);
		MenuBar::AddMenuItem("Edit/Redo", Undo::DoRedo);

		GIZMO_MENU("Gizmos/Operation/Translate", Gizmos::m_DefaultOperation, ImGuizmo::TRANSLATE);
		GIZMO_MENU("Gizmos/Operation/Rotate", Gizmos::m_DefaultOperation, ImGuizmo::ROTATE);
		GIZMO_MENU("Gizmos/Operation/Scale", Gizmos::m_DefaultOperation, ImGuizmo::SCALE);
		GIZMO_MENU("Gizmos/Operation/Universal", Gizmos::m_DefaultOperation, ImGuizmo::UNIVERSAL);

		GIZMO_MENU("Gizmos/Mode/Local", Gizmos::m_DefaultMode, ImGuizmo::LOCAL);
		GIZMO_MENU("Gizmos/Mode/World", Gizmos::m_DefaultMode, ImGuizmo::WORLD);
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
