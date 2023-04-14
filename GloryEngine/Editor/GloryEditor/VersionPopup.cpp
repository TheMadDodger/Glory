#include "VersionPopup.h"

namespace Glory::Editor
{
	std::string currentLatestVersion;
	bool openPopup = false;
	const char* popupName = "New Release Available";

	void VersionPopup::Open(const std::string& latestVersion)
	{
		currentLatestVersion = latestVersion;
		openPopup = true;
	}

	void VersionPopup::Draw()
	{
		static bool isOpen = false;

		if (openPopup)
		{
			ImGui::OpenPopup(popupName);
			isOpen = true;
			openPopup = false;
		}

		if (ImGui::BeginPopupModal(popupName, &isOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextUnformatted("A new Editor version is available!");
			ImGui::Text("Currently using %s but lastest available version is %s", GloryEditorVersion, currentLatestVersion);
			ImGui::Separator();

			const ImVec2 availableRegion = ImGui::GetContentRegionAvail();
			if (ImGui::Button("OK", { availableRegion.x / 2.0f - 5.0f, 0.0f }))
			{
				ImGui::CloseCurrentPopup();
				isOpen = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Download", { availableRegion.x / 2.0f - 5.0f, 0.0f }))
			{
				ImGui::CloseCurrentPopup();
				isOpen = false;
				/* TODO: Open download URL */
				system("start \"\" \"www.google.com\"");
			}

			ImGui::EndPopup();
		}
	}
}