#include "ProjectSettingsWindow.h"

namespace Glory::Editor
{
	ProjectSettingsWindow::ProjectSettingsWindow() : EditorWindowTemplate("Project Settings", 400.0f, 600.0f), m_CurrentTab(0)
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
			for (size_t i = 0; i < ProjectSettings::SettingsCount(); ++i)
			{
				if (ImGui::BeginTabItem(ProjectSettings::Name(i).data()))
				{
					if (m_CurrentTab != i)
					{
						m_CurrentTab = i;
						ProjectSettings::Open(m_CurrentTab);
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
		ProjectSettings::Open(m_CurrentTab);
	}
}
