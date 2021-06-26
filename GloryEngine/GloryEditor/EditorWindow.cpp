#include "stdafx.h"
#include "EditorWindow.h"

namespace Glory::Editor
{
	std::vector<EditorWindow*> EditorWindow::m_pActiveEditorWindows = std::vector<EditorWindow*>();
	std::vector<EditorWindow*> EditorWindow::m_pClosingEditorWindows = std::vector<EditorWindow*>();
	std::list<size_t> EditorWindow::m_IDs = std::list<size_t>();

	EditorWindow::EditorWindow(const std::string& windowName, float windowWidth, float windowHeight) : m_WindowName(windowName), m_WindowDimensions(windowWidth, windowHeight), m_IsOpen(true), m_Resizeable(true)
	{
	}

	EditorWindow::~EditorWindow()
	{
	}

	void EditorWindow::Close()
	{
		m_pClosingEditorWindows.push_back(this);
	}

	void EditorWindow::RenderGUI()
	{
		OnPaint();

		ImGuiWindowFlags window_flags = m_Resizeable? 0 : ImGuiWindowFlags_::ImGuiWindowFlags_NoResize;
		ImGui::SetNextWindowSize(m_WindowDimensions);

		std::string windowString = m_WindowName + "##" + std::to_string(m_WindowID);

		if (ImGui::Begin(windowString.c_str(), &m_IsOpen, window_flags))
		{
			OnGUI();
			m_WindowDimensions = ImGui::GetWindowSize();
		}
		ImGui::End();

		if (!m_IsOpen) Close();
	}

	void EditorWindow::RenderWindows()
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

	void EditorWindow::Cleanup()
	{
		std::for_each(m_pActiveEditorWindows.begin(), m_pActiveEditorWindows.end(), [&](EditorWindow* pWindow)
			{
				delete pWindow;
			});

		m_pActiveEditorWindows.clear();
		m_pClosingEditorWindows.clear();
		m_IDs.clear();
	}
}