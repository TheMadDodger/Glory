#include <imgui.h>
#include "MainEditor.h"
#include "EditorWindow.h"
#include "GameWindow.h"
#include "SceneWindow.h"
#include "InspectorWindow.h"
#include "SceneGraphWindow.h"
#include "ContentBrowser.h"
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
#include <Game.h>
#include <Engine.h>
#include <MaterialEditor.h>
#include "StandardPropertyDrawers.h"
#include <MaterialInstanceEditor.h>
#include <Serializer.h>
#include "EditorSceneManager.h"
#include <AssetDatabase.h>

namespace Glory::Editor
{
	MainEditor::MainEditor() : m_pAssetLoader(new EditorAssetLoader()), m_pProjectPopup(new ProjectPopup())
	{
	}

	MainEditor::~MainEditor()
	{
		delete m_pAssetLoader;
		m_pAssetLoader = nullptr;
	}

	void MainEditor::Initialize()
	{
		RegisterWindows();
		RegisterPropertyDrawers();
		RegisterEditors();

		CreateDefaultMainMenuBar();

		SetDarkThemeColors();

		m_pProjectPopup->Initialize();
		m_pProjectPopup->Open();

		Tumbnail::AddGenerator<TextureTumbnailGenerator>();
		Tumbnail::AddGenerator<SceneTumbnailGenerator>();
	}

	void MainEditor::Destroy()
	{
		ProjectSpace::CloseProject();
		EditorWindow::Cleanup();
		PropertyDrawer::Cleanup();
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
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        EditorWindow::RenderWindows();
		PopupManager::OnGUI();
		m_pProjectPopup->OnGui();
	}

	EditorAssetLoader* MainEditor::GetAssetLoader()
	{
		return m_pAssetLoader;
	}

	void MainEditor::CreateDefaultMainMenuBar()
	{
		MenuBar::AddMenuItem("File/New/Scene", EditorSceneManager::NewScene);
		MenuBar::AddMenuItem("File/Save Scene", EditorSceneManager::SaveOpenScenes);
		MenuBar::AddMenuItem("File/Load Scene", []()
			{
				//YAML::Node node = YAML::LoadFile("test.gscene");
				//Serializer::DeserializeObject(node);
			});

		MenuBar::AddMenuItem("File/Preferences", []() { EditorWindow::GetWindow<EditorPreferencesWindow>(); });
		MenuBar::AddMenuItem("File/Save Project", AssetDatabase::Save);
		MenuBar::AddMenuItem("File/Create/Empty Object", []()
			{
				GScene* pActiveScene = Game::GetGame().GetEngine()->GetScenesModule()->GetActiveScene();
				if (!pActiveScene) return;
				pActiveScene->CreateEmptyObject();
			});
		MenuBar::AddMenuItem("Play/Start", [&]() {/*this->EnterPlayMode();*/ });
		MenuBar::AddMenuItem("Play/Pauze", [&]() {/*m_PlayModePaused = !m_PlayModePaused;*/ });
		MenuBar::AddMenuItem("Play/Stop", [&]() {/*this->ExitPlayMode();*/ });

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
	}

	void MainEditor::RegisterEditors()
	{
		Editor::RegisterEditor<MaterialEditor>();
		Editor::RegisterEditor<MaterialInstanceEditor>();
	}
}