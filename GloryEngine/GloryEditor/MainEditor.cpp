#include "stdafx.h"
#include "MainEditor.h"
#include "EditorWindow.h"
#include "GameWindow.h"
#include "SceneWindow.h"
#include "InspectorWindow.h"
#include "SceneGraphWindow.h"
#include "ContentBrowser.h"
#include "MenuBar.h"
#include "PopupManager.h"
#include "EditorPreferencesWindow.h"
#include "Game.h"
#include "Window.h"
#include "PopupManager.h"

namespace Glory::Editor
{
	MainEditor::MainEditor()
	{
	}

	MainEditor::~MainEditor()
	{
	}

	void MainEditor::Initialize()
	{
		EditorWindow::GetWindow<GameWindow>();
		EditorWindow::GetWindow<SceneWindow>();
		EditorWindow::GetWindow<InspectorWindow>();
		EditorWindow::GetWindow<SceneGraphWindow>();
		EditorWindow::GetWindow<ContentBrowser>();

		CreateDefaultMainMenuBar();
	}

	void MainEditor::Destroy()
	{
		EditorWindow::Cleanup();
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

	void MainEditor::Paint()
	{
		MenuBar::OnGUI();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        EditorWindow::RenderWindows();
		PopupManager::OnGUI();
	}

	void MainEditor::CreateDefaultMainMenuBar()
	{
		MenuBar::AddMenuItem("File/New/Scene", NULL);
		MenuBar::AddMenuItem("File/Preferences", []() { EditorWindow::GetWindow<Editor::EditorPreferencesWindow>(); });
		MenuBar::AddMenuItem("File/Save Project", []() {/*AssetDatabase::SaveAssets();*/ });
		MenuBar::AddMenuItem("Play/Start", [&]() {/*this->EnterPlayMode();*/ });
		MenuBar::AddMenuItem("Play/Pauze", [&]() {/*m_PlayModePaused = !m_PlayModePaused;*/ });
		MenuBar::AddMenuItem("Play/Stop", [&]() {/*this->ExitPlayMode();*/ });

		MenuBar::AddMenuItem("File/Exit", [&]() {
			std::vector<std::string> buttons = { "Cancel", "Exit" };
			std::vector<std::function<void()>> buttonFuncs = { [&]() { Editor::PopupManager::CloseCurrentPopup(); }, [&]() {/*m_IsRunning = false;*/ } };
			PopupManager::OpenPopup("Exit", "Are you sure you want to exit? All unsaved changes will be lost!",
				buttons, buttonFuncs); });

		MenuBar::AddMenuItem("Window/Scene View", []() { Editor::EditorWindow::GetWindow<Editor::SceneWindow>(); });
		MenuBar::AddMenuItem("Window/Game View", []() { Editor::EditorWindow::GetWindow<Editor::GameWindow>(); });
		MenuBar::AddMenuItem("Window/Scene Graph", []() { Editor::EditorWindow::GetWindow<Editor::SceneGraphWindow>(); });
		MenuBar::AddMenuItem("Window/Inspector", []() { Editor::EditorWindow::GetWindow<Editor::InspectorWindow>(true); });
		MenuBar::AddMenuItem("Window/Content Browser", []() { Editor::EditorWindow::GetWindow<Editor::ContentBrowser>(); });
	}
}