#include "MainWindow.h"
#include "Toolbar.h"
#include "MainEditor.h"
#include "ImGuiHelpers.h"
#include "EditorWindow.h"

namespace Glory::Editor
{
    MainWindow::MainWindow()
    {
    }

    void MainWindow::DrawGui(float height)
    {
        OnGui(height);
        RenderWindows();
    }

    MainWindow::~MainWindow()
    {
        std::for_each(m_pActiveEditorWindows.begin(), m_pActiveEditorWindows.end(), [&](EditorWindow* pWindow)
        {
            pWindow->OnClose();
            delete pWindow;
        });

        m_pActiveEditorWindows.clear();
        m_pClosingEditorWindows.clear();
        m_IDs.clear();
    }

    void MainWindow::RenderWindows()
    {
        std::for_each(m_pClosingEditorWindows.begin(), m_pClosingEditorWindows.end(), [&](EditorWindow* pWindow)
        {
            auto it = std::find(m_pActiveEditorWindows.begin(), m_pActiveEditorWindows.end(), pWindow);
            m_pActiveEditorWindows.erase(it);
            m_IDs.push_back(pWindow->m_WindowID);
            delete pWindow;
        });

        m_pClosingEditorWindows.clear();

        std::for_each(m_pActiveEditorWindows.begin(), m_pActiveEditorWindows.end(), [&](EditorWindow* pWindow)
        {
            pWindow->RenderGUI();
        });
    }

    EditorWindow* MainWindow::FindEditorWindow(const std::type_info& type)
    {
        for (size_t i = 0; i < m_pActiveEditorWindows.size(); ++i)
        {
            if (m_pActiveEditorWindows[i]->GetType() == type)
                return m_pActiveEditorWindows[i];
        }
        return nullptr;
    }

    void MainWindow::OpenEditorWindow(EditorWindow* pWindow)
    {
        m_pActiveEditorWindows.push_back(pWindow);

        if (m_IDs.size() > 0)
        {
            pWindow->m_WindowID = m_IDs.front();
            m_IDs.pop_front();
        }
        else pWindow->m_WindowID = m_pActiveEditorWindows.size();

        pWindow->OnOpen();
    }

    void MainWindow::UpdateWindows()
    {
        for (size_t i = 0; i < m_pActiveEditorWindows.size(); ++i)
        {
            m_pActiveEditorWindows[i]->Update();
            m_pActiveEditorWindows[i]->Draw();
        }
    }

    void MainWindow::Dockspace(float height)
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
    }

    void SceneEditingMainWindow::Initialize()
    {
    }
}
