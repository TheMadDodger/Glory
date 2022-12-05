#include <imgui.h>
#include "EditorPreferencesWindow.h"

namespace Glory::Editor
{
	int EditorPreferencesWindow::m_ThemeIndex = 0;

	EditorPreferencesWindow::EditorPreferencesWindow() : EditorWindowTemplate("Preferences", 600.0f, 400.0f)
	{
		m_Resizeable = false;
	}

	EditorPreferencesWindow::~EditorPreferencesWindow()
	{
	}

	void EditorPreferencesWindow::OnGUI()
	{
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("Preferences Main Tab", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("Appearance"))
			{
				AppearanceTab();
				ImGui::EndTabItem();
			}
			//if (ImGui::BeginTabItem("Broccoli"))
			//{
			//	ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
			//	ImGui::EndTabItem();
			//}
			//if (ImGui::BeginTabItem("Cucumber"))
			//{
			//	ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
			//	ImGui::EndTabItem();
			//}
			ImGui::EndTabBar();
		}
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
}