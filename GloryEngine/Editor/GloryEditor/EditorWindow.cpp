#include "EditorWindow.h"

namespace Glory::Editor
{
	EditorWindow::EditorWindow(const std::string& windowName, float windowWidth, float windowHeight)
		: m_WindowName(windowName), m_WindowDimensions(windowWidth, windowHeight),
		m_IsOpen(true), m_Resizeable(true), m_WindowFlags(ImGuiWindowFlags_::ImGuiWindowFlags_None),
		m_IsFocused(false), m_WindowID(0)
	{
	}

	EditorWindow::~EditorWindow()
	{
	}

	void EditorWindow::Close()
	{
		OnClose();
		m_pOwner->m_pClosingEditorWindows.push_back(this);
	}

	const bool EditorWindow::IsFocused() const
	{
		return m_IsFocused;
	}

	void EditorWindow::RenderGUI()
	{
		ImGuiWindowFlags window_flags = m_WindowFlags | (m_Resizeable? 0 : ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);
		ImGui::SetNextWindowSize(m_WindowDimensions);

		const std::string windowString = m_WindowName + "##" + std::to_string(m_WindowID);

		if (ImGui::Begin(windowString.c_str(), &m_IsOpen, window_flags))
		{
			m_IsFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
			OnGUI();
			m_WindowDimensions = ImGui::GetWindowSize();
		}
		else m_IsFocused = false;
		ImGui::End();
		m_DragAndDrop.HandleDragAndDropTarget([this](uint32_t type, const ImGuiPayload* payload) { HandleDragAndDropPayload(type, payload); });

		if (!m_IsOpen) Close();
	}
}