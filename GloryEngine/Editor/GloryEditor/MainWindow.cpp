#include "MainWindow.h"
#include "Toolbar.h"
#include "MainEditor.h"
#include "ImGuiHelpers.h"
#include "EditorWindow.h"

namespace Glory::Editor
{
    const float SceneEditingMainWindow::TOOLBAR_SIZE = 50.0f;

    SceneEditingMainWindow::SceneEditingMainWindow(): m_pToolbar(new Toolbar(TOOLBAR_SIZE))
    {
    }

    SceneEditingMainWindow::~SceneEditingMainWindow()
    {
        delete m_pToolbar;
        m_pToolbar = nullptr;
    }

    std::string_view SceneEditingMainWindow::Name()
    {
        return "Scene Editing";
    }

    void SceneEditingMainWindow::OnGui(float height)
    {
        m_pToolbar->Paint();
        Dockspace(height + TOOLBAR_SIZE);
        EditorWindow::RenderWindows();
    }

    void SceneEditingMainWindow::Initialize()
    {
    }

    void SceneEditingMainWindow::Dockspace(float height)
    {
        const ImGuiID dockSpaceId = ImGui::GetID("Scene Editing Space");
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos + ImVec2(0, height));
        ImGui::SetNextWindowSize(viewport->Size - ImVec2(0, height));
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags window_flags = 0
            | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin(Name().data(), NULL, window_flags);

        ImGui::DockSpace(dockSpaceId);
        ImGui::End();
        ImGui::PopStyleVar(3);
    }

    std::string_view TestMainWindow::Name()
    {
        return "Test";
    }

    void TestMainWindow::OnGui(float height)
    {
        const ImGuiID dockSpaceId = ImGui::GetID("Scene Editing Space");
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos + ImVec2(0, height));
        ImGui::SetNextWindowSize(viewport->Size - ImVec2(0, height));
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags window_flags = 0
            | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin(Name().data(), NULL, window_flags);

        ImGui::DockSpace(dockSpaceId);
        ImGui::End();
        ImGui::PopStyleVar(3);
    }

    void TestMainWindow::Initialize()
    {
    }
}
