#include "PackagePopup.h"
#include "Package.h"
#include "Dispatcher.h"
#include "ProjectSpace.h"

#include <imgui.h>
#include <sstream>

namespace Glory::Editor
{
	PackagePopup::PackagePopup()
	{
		m_PackagingStart = PackagingStartedEvent().AddListener([this](const EmptyEvent&) {
			m_Open = true;
			m_Canceling = false;
		});
	}

	PackagePopup::~PackagePopup()
	{
		PackagingStartedEvent().RemoveListener(m_PackagingStart);
	}

	void PackagePopup::Draw()
	{
		if (m_Open)
		{
			ImGui::OpenPopup("Packaging");
			m_Open = false;
		}

		ImGui::SetNextWindowSize({ 400.0f, 0.0f });
		if (ImGui::BeginPopupModal("Packaging", nullptr, ImGuiWindowFlags_NoResize))
		{
			size_t currentIndex, count, subIndex, subCount;
			std::string_view name, subName;

			const bool done = PackageState(currentIndex, count, name, subIndex, subCount, subName);

			if (done && m_Canceling)
				ImGui::CloseCurrentPopup();

			ImGui::Text("%s: %i/%i", name.data(), currentIndex, count);
			const float percentage = float(currentIndex)/count;
			ImGui::ProgressBar(percentage);

			ImGui::Text("%s: %i/%i", subName.data(), subIndex, subCount);
			const float subPercentage = subCount != 0 ? float(subIndex)/subCount : 1.0f;
			ImGui::ProgressBar(subPercentage);

			const float availableWidth = ImGui::GetContentRegionAvail().x;
			const float buttonWidth = availableWidth / 4.0f;

			if (done)
			{
				const float posX = availableWidth - buttonWidth*2.0f;
				ImGui::SetCursorPosX(posX);

				if (ImGui::Button("Play", { buttonWidth, 0.0f }))
				{
					ProjectSpace* pProject = ProjectSpace::GetOpenProject();
					std::filesystem::path packageRoot = pProject->RootPath();
					packageRoot.append("Build");
					std::stringstream str;
					str << "cd " << packageRoot.string() << " && " << pProject->Name() << ".exe";
					system(str.str().data());
				}
				ImGui::SameLine();

				if (ImGui::Button("Ok", { buttonWidth, 0.0f }))
				{
					ImGui::CloseCurrentPopup();
				}
			}
			else
			{
				ImGui::BeginDisabled(m_Canceling);
				const float posX = availableWidth - buttonWidth + 8.0f;
				ImGui::SetCursorPosX(posX);
				if (ImGui::Button("Cancel", { buttonWidth, 0.0f }))
				{
					CancelPackage();
					m_Canceling = true;
				}
				ImGui::EndDisabled();
			}

			ImGui::EndPopup();
		}
	}
}
