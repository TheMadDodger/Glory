#include "RemovedAssetsPopup.h"
#include "AssetLocation.h"

#include <imgui.h>
#include <ProjectSpace.h>

#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	std::vector<AssetLocation> RemovedAssetsPopup::m_RemovedAssets;

    void RemovedAssetsPopup::AddRemovedAssets(std::vector<AssetLocation>&& assets)
    {
        const size_t index = m_RemovedAssets.size();
        m_RemovedAssets.reserve(m_RemovedAssets.size() + assets.size());
		for (size_t i = 0; i < assets.size(); ++i)
		{
			m_RemovedAssets.push_back(std::move(assets[i]));
		}
    }

    void RemovedAssetsPopup::Draw()
    {
		static bool open = false;
		if (m_RemovedAssets.size())
		{
			ImGui::OpenPopup(ICON_FA_TRIANGLE_EXCLAMATION " Missing assets");
			open = true;
		}

		const bool wasOpen = open;
		if (ImGui::BeginPopupModal(ICON_FA_TRIANGLE_EXCLAMATION " Missing assets", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
		{
			//ImGui::TextUnformatted("Are you sure you want to quit?");
			ImGui::TextUnformatted("The following assets can no longer be found and have been removed:");
			ImGui::Separator();

			ImGui::BeginChild("Body", { 500.0f, 200.0f });
			if (ImGui::BeginListBox("BodyList", ImGui::GetContentRegionAvail()))
			{
				int index = 0;
				for (const auto& location : m_RemovedAssets)
				{
					ImGui::PushID(index);
					ImGui::Selectable("##selectable", false, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_DontClosePopups);
					ImGui::SameLine();
					ImGui::Text("%s \\\\ %s", location.Path.data(), location.SubresourcePath.data());
					ImGui::PopID();
					++index;
				}
				ImGui::EndListBox();
			}
			ImGui::EndChild();
			ImGui::TextUnformatted("Things may break if any of these were referenced.\n"
				"If you save the project these changes cannot be undone.");

			const ImVec2 availableRegion = ImGui::GetContentRegionAvail();
			if (ImGui::Button("Save now", { availableRegion.x / 2.0f - 5.0f, 0.0f }))
			{
				ProjectSpace::Save();
				ImGui::CloseCurrentPopup();
				open = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Ok", { availableRegion.x / 2.0f - 5.0f, 0.0f }))
			{
				ImGui::CloseCurrentPopup();
				open = false;
			}

			ImGui::EndPopup();
		}

		if (!open && wasOpen)
		{
			m_RemovedAssets.clear();
		}
    }
}
