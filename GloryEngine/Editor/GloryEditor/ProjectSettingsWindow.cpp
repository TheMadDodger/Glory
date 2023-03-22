#include "ProjectSettingsWindow.h"

namespace Glory::Editor
{
	const char* ProjectSettingsWindow::TAB_NAMES[] = {
		"General", "Engine", "Layers", "Input", "Physics"
	};

	ProjectSettingsWindow::ProjectSettingsWindow() : EditorWindowTemplate("Project Settings", 400.0f, 600.0f), m_CurrentTab(ProjectSettingsType::General)
	{
		m_Resizeable = true;
	}

	ProjectSettingsWindow::~ProjectSettingsWindow()
	{
	}

	void ProjectSettingsWindow::OnGUI()
	{
		if (ImGui::BeginTabBar("projectsettingstabs"))
		{
			for (size_t i = 0; i < size_t(ProjectSettingsType::Count); i++)
			{
				ProjectSettingsType tab = ProjectSettingsType(i);
				if (ImGui::BeginTabItem(TAB_NAMES[i]))
				{
					if (m_CurrentTab != tab)
					{
						m_CurrentTab = tab;
					}
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}

		ProjectSettings::Paint(m_CurrentTab);
	}

	void ProjectSettingsWindow::OnOpen()
	{
	}
}
