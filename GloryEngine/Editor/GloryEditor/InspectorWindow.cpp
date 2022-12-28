#include <imgui.h>
#include <AssetDatabase.h>
#include "InspectorWindow.h"
#include "Selection.h"
#include "Editor.h"

#include "FontAwesome/IconsFontAwesome6.h"

namespace Glory::Editor
{
	InspectorWindow::InspectorWindow() : EditorWindowTemplate("Inspector", 300.0f, 680.0f), m_Locked(false), m_pCurrentObject(nullptr), m_pEditor(nullptr),
		m_SelectionCallbackID(Selection::SubscribeToSelectionChange([&]() { OnSelectionChange(); })) {}

	InspectorWindow::~InspectorWindow()
	{
		Selection::UnsubscribeToSelectionChange(m_SelectionCallbackID);
	}

	void InspectorWindow::OnGUI()
	{
		Object* pSelectedObject = Selection::GetActiveObject();

		if (pSelectedObject != m_pCurrentObject && !m_Locked)
		{
			if (m_pEditor) Editor::ReleaseEditor(m_pEditor);
			m_pCurrentObject = pSelectedObject;
			CreateEditor();
		}

		const std::string_view icon = m_Locked ? ICON_FA_LOCK : ICON_FA_LOCK_OPEN;
		const ImVec2 textSize = ImGui::CalcTextSize(icon.data());
		const float buttonSize = textSize.x + 4;

		const ImVec2 cursorPos = ImGui::GetCursorPos();
		ImGui::PushStyleColor(ImGuiCol_Header, { 0,0,0,0 });
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0,0,0,0 });
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 0,0,0,0 });
		if (ImGui::Selectable("##lock", false, 0, ImVec2(buttonSize, buttonSize)))
			m_Locked = !m_Locked;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::SetCursorPos({ cursorPos.x + buttonSize / 2 - textSize.x / 2, cursorPos.y + buttonSize / 2 - textSize.y / 2 });
		if (ImGui::IsItemHovered())
			ImGui::TextColored({ 0.9f, 0.9f, 0.9f, 1.0f }, icon.data());
		else
			ImGui::TextUnformatted(icon.data());

		ImGui::Spacing();

		if (m_pEditor)
		{
			if (m_pEditor->OnGUI()) AssetDatabase::SetDirty(pSelectedObject);
		}
	}

	void InspectorWindow::CreateEditor()
	{
		if (!m_pCurrentObject)
		{
			if (m_pEditor) Editor::ReleaseEditor(m_pEditor);
			m_pEditor = nullptr;
			return;
		}
		m_pEditor = Editor::CreateEditor(m_pCurrentObject);
	}

	void InspectorWindow::OnSelectionChange()
	{
		Object* pSelectedObject = Selection::GetActiveObject();

		if (pSelectedObject != m_pCurrentObject && !m_Locked)
		{
			if (m_pEditor) Editor::ReleaseEditor(m_pEditor);
			m_pCurrentObject = pSelectedObject;
			CreateEditor();
		}
	}
}