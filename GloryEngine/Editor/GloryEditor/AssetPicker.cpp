#include "AssetPicker.h"
#include "EditorUI.h"

#include <imgui.h>
#include <algorithm>
#include <EditorAssetDatabase.h>
#include <Engine.h>
#include <WindowModule.h>

#include <IconsFontAwesome6.h>

namespace Glory::Editor
{
	char AssetPicker::m_FilterBuffer[200] = "";
	std::string AssetPicker::m_Filter = "";
	std::vector<UUID> AssetPicker::m_FilteredAssets;
	std::vector<UUID> AssetPicker::m_PossibleAssets;

	bool AssetPicker::ResourceDropdown(const std::string& label, uint32_t resourceType, UUID* value, bool includeSubAssets, const float borderPadding)
	{
		ImGui::PushID(label.c_str());
		std::string assetName = "";
		assetName = EditorAssetDatabase::GetAssetName(*value);
		if (assetName == "") assetName = "Noone";

		bool openPopup = false;
		float start, width;
		EditorUI::EmptyDropdown(EditorUI::MakeCleanName(label), assetName, [&]
		{
			m_PossibleAssets.clear();
			m_FilteredAssets.clear();
			openPopup = true;
		}, start, width, borderPadding);

		if (openPopup)
			ImGui::OpenPopup("AssetPicker");
		openPopup = false;

		const ImVec2 cursor = ImGui::GetCursorPos();
		const ImVec2 windowPos = ImGui::GetWindowPos();
		Window* pWindow = Game::GetGame().GetEngine()->GetMainModule<WindowModule>()->GetMainWindow();
		int mainWindowWidth, mainWindowHeight;
		pWindow->GetDrawableSize(&mainWindowWidth, &mainWindowHeight);
		ImGui::SetNextWindowPos({ windowPos.x + start, windowPos.y + cursor.y - 2.5f });
		ImGui::SetNextWindowSize({ width, mainWindowHeight - windowPos.y - cursor.y - 10.0f });
		bool change = DrawPopup(value, resourceType, includeSubAssets);
		ImGui::PopID();
		return change;
	}

	bool AssetPicker::ResourceButton(const std::string& label, float buttonWidth, uint32_t resourceType, UUID* value, bool includeSubAssets)
	{
		ImGui::PushID(label.c_str());
		bool openPopup = false;
		if (ImGui::Button(label.c_str(), ImVec2(buttonWidth, 0.0f)))
		{
			m_PossibleAssets.clear();
			m_FilteredAssets.clear();
			openPopup = true;
		}
		if (openPopup)
		{
			ImGui::OpenPopup("AssetPicker");
		}
		openPopup = false;

		const bool change = DrawPopup(value, resourceType, includeSubAssets);
		ImGui::PopID();
		return change;
	}

	AssetPicker::AssetPicker() {}
	AssetPicker::~AssetPicker() {}

	void AssetPicker::RefreshFilter()
	{
		m_FilteredAssets.clear();
		if (m_Filter == "") return;

		size_t compCount = 0;
		for (size_t i = 0; i < m_PossibleAssets.size(); i++)
		{
			UUID uuid = m_PossibleAssets[i];
			std::string name = EditorAssetDatabase::GetAssetName(uuid);
			if (name.find(m_Filter) == std::string::npos) continue;
			m_FilteredAssets.push_back(m_PossibleAssets[i]);
		}
	}

	void AssetPicker::LoadAssets(uint32_t typeHash, bool includeSubAssets)
	{
		m_PossibleAssets.clear();
		EditorAssetDatabase::GetAllAssetsOfType(typeHash, m_PossibleAssets);

		if (!includeSubAssets) return;
		std::vector<ResourceType*> pTypes;
		ResourceType::GetAllResourceTypesThatHaveSubType(typeHash, pTypes);
		for (size_t i = 0; i < pTypes.size(); i++)
		{
			if (pTypes[i]->Hash() == typeHash) continue;
			EditorAssetDatabase::GetAllAssetsOfType(pTypes[i]->Hash(), m_PossibleAssets);
		}
	}

	bool AssetPicker::DrawPopup(UUID* value, uint32_t typeHash, bool includeSubAssets)
	{
		bool assetChosen = false;

		if (ImGui::BeginPopup("AssetPicker"))
		{
			if (m_PossibleAssets.size() == 0)
				LoadAssets(typeHash, includeSubAssets);

			//ImGui::Text("Asset Picker");
			ImGui::TextUnformatted(ICON_FA_MAGNIFYING_GLASS);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputText("##search", m_FilterBuffer, 200);

			if (m_Filter != std::string(m_FilterBuffer))
			{
				m_Filter = std::string(m_FilterBuffer);
				RefreshFilter();
			}

			if (m_Filter.length() > 0)
			{
				assetChosen = DrawItems(m_FilteredAssets, value);
			}
			else
			{
				assetChosen = DrawItems(m_PossibleAssets, value);
			}

			ImGui::EndPopup();
		}

		if (assetChosen)
		{
			m_Filter = "";
			m_FilteredAssets.clear();
			m_PossibleAssets.clear();
			ImGui::CloseCurrentPopup();
		}
		return assetChosen;
	}

	bool AssetPicker::DrawItems(const std::vector<UUID>& items, UUID* value)
	{
		bool change = false;
		if (ImGui::MenuItem("Noone"))
		{
			*value = 0;
			m_PossibleAssets.clear();
			m_FilteredAssets.clear();
			change = true;
		}

		std::for_each(items.begin(), items.end(), [&](UUID uuid)
		{
			const std::string name = EditorAssetDatabase::GetAssetName(uuid);
			if (ImGui::MenuItem(name.c_str()))
			{
				*value = uuid;
				m_PossibleAssets.clear();
				m_FilteredAssets.clear();
				change = true;
			}
		});
		return change;
	}
}