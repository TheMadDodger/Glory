#include <imgui.h>
#include "EditorPreferencesWindow.h"
#include "Shortcuts.h"

namespace Glory::Editor
{
	const size_t ALLOWED_SHORTCUT_KEYS_COUNT = 48;
	const ImGuiKey ALLOWED_SHORTCUT_KEYS[ALLOWED_SHORTCUT_KEYS_COUNT] = { ImGuiKey_0, ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, ImGuiKey_4, ImGuiKey_5, ImGuiKey_6, ImGuiKey_7, ImGuiKey_8, ImGuiKey_9,
	ImGuiKey_A, ImGuiKey_B, ImGuiKey_C, ImGuiKey_D, ImGuiKey_E, ImGuiKey_F, ImGuiKey_G, ImGuiKey_H, ImGuiKey_I, ImGuiKey_J,
	ImGuiKey_K, ImGuiKey_L, ImGuiKey_M, ImGuiKey_N, ImGuiKey_O, ImGuiKey_P, ImGuiKey_Q, ImGuiKey_R, ImGuiKey_S, ImGuiKey_T,
	ImGuiKey_U, ImGuiKey_V, ImGuiKey_W, ImGuiKey_X, ImGuiKey_Y, ImGuiKey_Z,
	ImGuiKey_F1, ImGuiKey_F2, ImGuiKey_F3, ImGuiKey_F4, ImGuiKey_F5, ImGuiKey_F6,
	ImGuiKey_F7, ImGuiKey_F8, ImGuiKey_F9, ImGuiKey_F10, ImGuiKey_F11, ImGuiKey_F12, };

	int EditorPreferencesWindow::m_ThemeIndex = 0;
	std::vector<PreferencesTab> EditorPreferencesWindow::m_Tabs;

	EditorPreferencesWindow::EditorPreferencesWindow()
		: EditorWindowTemplate("Preferences", 600.0f, 400.0f), m_RebindingShortcut("")
	{
		m_Resizeable = true;
	}

	EditorPreferencesWindow::~EditorPreferencesWindow()
	{
	}

	void EditorPreferencesWindow::AddPreferencesTab(const PreferencesTab&& tab)
	{
		m_Tabs.push_back(std::move(tab));
	}

	void EditorPreferencesWindow::OnGUI()
	{
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("Preferences Main Tab", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("Shortcuts"))
			{
				Shortcuts();
				ImGui::EndTabItem();
			}

			for (size_t i = 0; i < m_Tabs.size(); ++i)
			{
				if (ImGui::BeginTabItem(m_Tabs[i].m_Name.c_str()))
				{
					m_Tabs[i].m_DrawCallback();
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}
	}

	void EditorPreferencesWindow::OnClose()
	{
		m_RebindingShortcut = "";
	}

	void EditorPreferencesWindow::UpdateTheme()
	{
		switch (m_ThemeIndex)
		{
		case 0:
			ImGui::StyleColorsDark();
			break;
		case 1:
			ImGui::StyleColorsClassic();
			break;
		case 2:
			ImGui::StyleColorsLight();
			break;
		default:
			break;
		}
	}

	void EditorPreferencesWindow::AppearanceTab()
	{
		int current = m_ThemeIndex;
		ImGui::Combo("Editor Skin", &m_ThemeIndex, "Dark\0Classic\0Light\0\0");
		if (current != m_ThemeIndex) UpdateTheme();

		ImGui::Separator();

		ImGui::Text("Rounding");
		ImGui::SliderFloat("Window Rounding", &ImGui::GetStyle().WindowRounding, 0.0f, 10.0f);
		ImGui::SliderFloat("Child Rounding", &ImGui::GetStyle().ChildRounding, 0.0f, 10.0f);
		ImGui::SliderFloat("Frame Rounding", &ImGui::GetStyle().FrameRounding, 0.0f, 10.0f);
		ImGui::SliderFloat("Grab Rounding", &ImGui::GetStyle().GrabRounding, 0.0f, 10.0f);
		ImGui::SliderFloat("Popup Rounding", &ImGui::GetStyle().PopupRounding, 0.0f, 10.0f);
		ImGui::SliderFloat("Scrollbar Rounding", &ImGui::GetStyle().ScrollbarRounding, 0.0f, 10.0f);
	}

	void EditorPreferencesWindow::Shortcuts()
	{
		if (ImGui::BeginListBox("##shortcutslist", ImGui::GetContentRegionAvail()))
		{
			for (auto itor = Shortcuts::Begin(); itor != Shortcuts::End(); itor++)
			{
				const Shortcut& shortcut = itor->second;
				const bool isRebinding = m_RebindingShortcut == itor->first;
				ImGui::Selectable(shortcut.m_Name, isRebinding);
				if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					m_RebindingShortcut = itor->first;

				const std::string shortcutString = isRebinding ? "..." : Shortcuts::GetShortcutString(itor->first);
				if (shortcutString.empty()) continue;
				ImGui::SameLine();
				const ImVec2 cursorPos = ImGui::GetCursorPos();
				const float availableWidth = ImGui::GetContentRegionAvail().x;
				const float textSize = ImGui::CalcTextSize(shortcutString.data()).x;
				ImGui::SetCursorPos({ cursorPos.x + availableWidth - textSize - ImGui::GetStyle().FramePadding.x, cursorPos.y });
				ImGui::TextDisabled(shortcutString.data());
			}
			ImGui::EndListBox();
		}

		if (!m_RebindingShortcut.empty())
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				/* Unbind */
				Shortcuts::SetShortcut(m_RebindingShortcut, ImGuiKey_None, ImGuiModFlags_None);
				return;
			}

			if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
			{
				/* Cancel */
				m_RebindingShortcut = "";
				return;
			}

			for (size_t i = 0; i < ALLOWED_SHORTCUT_KEYS_COUNT; i++)
			{
				if (!ImGui::IsKeyDown(ALLOWED_SHORTCUT_KEYS[i])) continue;
				/* Bind to current input */
				Shortcuts::SetShortcut(m_RebindingShortcut, ALLOWED_SHORTCUT_KEYS[i], ImGui::GetIO().KeyMods);
				m_RebindingShortcut = "";
				break;
			}
		}
	}
}