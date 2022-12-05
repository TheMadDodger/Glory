#include "EditorWindow.h"

namespace Glory::Editor
{
	std::vector<EditorWindow*> EditorWindow::m_pActiveEditorWindows = std::vector<EditorWindow*>();
	std::vector<EditorWindow*> EditorWindow::m_pClosingEditorWindows = std::vector<EditorWindow*>();
	std::list<size_t> EditorWindow::m_IDs = std::list<size_t>();

	GLORY_EDITOR_API EditorWindow::EditorWindow(const std::string& windowName, float windowWidth, float windowHeight)
		: m_WindowName(windowName), m_WindowDimensions(windowWidth, windowHeight),
		m_IsOpen(true), m_Resizeable(true), m_WindowFlags(ImGuiWindowFlags_::ImGuiWindowFlags_None)
	{
	}

	GLORY_EDITOR_API EditorWindow::~EditorWindow()
	{
	}

	GLORY_EDITOR_API EditorWindow* EditorWindow::FindEditorWindow(const std::type_info& type)
	{
		for (size_t i = 0; i < m_pActiveEditorWindows.size(); ++i)
		{
			if (m_pActiveEditorWindows[i]->GetType() == type)
				return m_pActiveEditorWindows[i];
		}
		return nullptr;
	}

	void EditorWindow::OpenEditorWindow(EditorWindow* pWindow)
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

	GLORY_EDITOR_API void EditorWindow::UpdateWindows()
	{
		for (size_t i = 0; i < m_pActiveEditorWindows.size(); i++)
		{
			m_pActiveEditorWindows[i]->Update();
			m_pActiveEditorWindows[i]->Draw();
		}
	}

	GLORY_EDITOR_API void EditorWindow::Close()
	{
		OnClose();
		m_pClosingEditorWindows.push_back(this);
	}

	void EditorWindow::RenderGUI()
	{
		ImGuiWindowFlags window_flags = m_WindowFlags | (m_Resizeable? 0 : ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);
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
				pWindow->OnClose();
				delete pWindow;
			});

		m_pActiveEditorWindows.clear();
		m_pClosingEditorWindows.clear();
		m_IDs.clear();
	}
}