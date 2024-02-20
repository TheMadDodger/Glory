#include "ProjectSettings.h"
#include "EditorUI.h"
#include "SettingsEnums.h"

#include <imgui.h>
#include <Reflection.h>

namespace Glory::Editor
{
	bool PackageSettings::OnGui()
	{
		ImGui::BeginChild("Packaging Settings");
		bool change = false;

		if (ImGui::CollapsingHeader("Scene Packaging"))
		{
			change |= EditorUI::InputEnum(m_YAMLFile, "Scenes/PackageScenesMode", ResourceTypes::GetHash<PackageScenes>());
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Which scenes to package:\n\n"
					              "- All will package all scenes in the project,\nthis is the default option\n"
					              "- Opened will package all scenes currently opened\n"
					              "- List will only package the scenes selected in a list"
				);
			}
		}

		ImGui::EndChild();
		return change;
	}

	void PackageSettings::OnSettingsLoaded()
	{
		auto packageScenesMode = RootValue()["Scenes/PackageScenesMode"];
		if (!packageScenesMode.Exists())
		{
			packageScenesMode.SetEnum(PackageScenes::All);
		}
	}
}
