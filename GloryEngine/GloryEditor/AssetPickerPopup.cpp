#include "AssetPickerPopup.h"
#include <imgui.h>
#include <algorithm>
#include <AssetDatabase.h>
#include <AssetManager.h>

namespace Glory::Editor
{
	bool AssetPickerPopup::m_Open = false;
	Resource** AssetPickerPopup::m_pResourcePointer = nullptr;
	size_t AssetPickerPopup::m_TypeHash = 0;
	bool AssetPickerPopup::m_IncludeSubAssets = false;
	std::function<void(Resource*)> AssetPickerPopup::m_Callback = NULL;

	void AssetPickerPopup::Open(size_t typeHash, Resource** pResource, bool includeSubAssets)
	{
		m_Open = true;
		m_TypeHash = typeHash;
		m_pResourcePointer = pResource;
		m_IncludeSubAssets = includeSubAssets;
	}

	void AssetPickerPopup::Open(size_t typeHash, std::function<void(Resource*)> callback, bool includeSubAssets)
	{
		m_Callback = callback;
		m_Open = true;
		m_TypeHash = typeHash;
		m_pResourcePointer = nullptr;
		m_IncludeSubAssets = includeSubAssets;
	}

	void AssetPickerPopup::OnGUI()
	{
		if (m_Open)
		{
			ImGui::OpenPopup("AssetPickerPopup");
			m_Filter = "";
			m_FilteredAssets.clear();
			m_PossibleAssets.clear();
		}
		m_Open = false;

		if (m_TypeHash == 0) return;

		if (m_PossibleAssets.size() == 0)
			LoadAssets();

		if (ImGui::BeginPopup("AssetPickerPopup"))
		{
			ImGui::Text("Asset Picker");

			ImGui::InputText("##", m_FilterBuffer, 200);

			if (m_Filter != std::string(m_FilterBuffer))
			{
				m_Filter = std::string(m_FilterBuffer);
				RefreshFilter();
			}

			if (m_Filter.length() > 0)
			{
				DrawItems(m_FilteredAssets);
			}
			else
			{
				DrawItems(m_PossibleAssets);
			}

			ImGui::EndPopup();
		}
	}

	AssetPickerPopup::AssetPickerPopup() : m_FilterBuffer("")
	{
	}

	AssetPickerPopup::~AssetPickerPopup()
	{
	}

	void AssetPickerPopup::RefreshFilter()
	{
		m_FilteredAssets.clear();
		if (m_Filter == "") return;
	
		size_t compCount = 0;
		for (size_t i = 0; i < m_PossibleAssets.size(); i++)
		{
			UUID uuid = m_PossibleAssets[i];
			std::string name = AssetDatabase::GetAssetName(uuid);
			if (name.find(m_Filter) == std::string::npos) continue;
			m_FilteredAssets.push_back(m_PossibleAssets[i]);
		}
	}

	void AssetPickerPopup::LoadAssets()
	{
		m_PossibleAssets.clear();
		AssetDatabase::GetAllAssetsOfType(m_TypeHash, m_PossibleAssets);

		if (!m_IncludeSubAssets) return;
		std::vector<ResourceType*> pTypes;
		ResourceType::GetAllResourceTypesThatHaveSubType(m_TypeHash, pTypes);
		for (size_t i = 0; i < pTypes.size(); i++)
		{
			if (pTypes[i]->Hash() == m_TypeHash) continue;
			AssetDatabase::GetAllAssetsOfType(pTypes[i]->Hash(), m_PossibleAssets);
		}
	}

	void AssetPickerPopup::DrawItems(const std::vector<UUID>& items)
	{
		if (ImGui::MenuItem("Noone"))
		{
			AssetSelected(nullptr);
		}

		std::for_each(items.begin(), items.end(), [&](UUID uuid)
		{
			std::string name = AssetDatabase::GetAssetName(uuid);
			if (ImGui::MenuItem(name.c_str()))
			{
				AssetManager::GetAsset(uuid, [&](Resource* pResource)
				{
					AssetSelected(pResource);
				});
			}
		});
	}

	void AssetPickerPopup::AssetSelected(Resource* pAsset)
	{
		m_PossibleAssets.clear();
		m_FilteredAssets.clear();
		ImGui::CloseCurrentPopup();

		if (m_Callback != NULL)
		{
			m_Callback(pAsset);
			m_Callback = NULL;
			return;
		}

		*m_pResourcePointer = pAsset;
		m_pResourcePointer = nullptr;
	}
}