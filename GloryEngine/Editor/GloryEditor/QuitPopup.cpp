#include "QuitPopup.h"
#include <imgui.h>
#include <EditorApplication.h>

namespace Glory::Editor
{
	bool QuitPopup::m_Open = false;
	std::string QuitPopup::m_Text;

	void QuitPopup::Open(const char* text)
	{
		m_Text = text;
		m_Open = true;
	}

	void QuitPopup::Draw()
	{
		if (m_Open)
		{
			ImGui::OpenPopup("Quit");
		}

		if (ImGui::BeginPopupModal("Quit", &m_Open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
		{
			//ImGui::TextUnformatted("Are you sure you want to quit?");
			ImGui::TextUnformatted(m_Text.c_str());
			ImGui::Separator();

			const ImVec2 availableRegion = ImGui::GetContentRegionAvail();
			if (ImGui::Button("No", { availableRegion.x / 2.0f - 5.0f, 0.0f }))
			{
				ImGui::CloseCurrentPopup();
				m_Open = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Yes", { availableRegion.x / 2.0f - 5.0f, 0.0f }))
			{
				EditorApplication::Quit();
				ImGui::CloseCurrentPopup();
				m_Open = false;
			}

			ImGui::EndPopup();
		}
	}
}
