//#include "AssetPickerPopup.h"
//
//namespace Spartan::Editor
//{
//	bool AssetPickerPopup::m_Open = false;
//	Spartan::Serialization::BaseAssetReference* AssetPickerPopup::m_pAssetReference = nullptr;
//
//	void AssetPickerPopup::Open(Spartan::Serialization::BaseAssetReference* pAssetReference)
//	{
//		m_Open = true;
//		m_pAssetReference = pAssetReference;
//	}
//
//	void AssetPickerPopup::OnGUI()
//	{
//		if (m_Open)
//			ImGui::OpenPopup("AssetPickerPopup");
//		m_Open = false;
//
//		if (!m_pAssetReference) return;
//
//		if (m_pPossibleAssets.size() == 0)
//			LoadAssets();
//
//		if (ImGui::BeginPopup("AssetPickerPopup"))
//		{
//			ImGui::Text("Asset Picker");
//
//			ImGui::InputText("", m_FilterBuffer, 200);
//
//			if (m_Filter != std::string(m_FilterBuffer))
//			{
//				m_Filter = std::string(m_FilterBuffer);
//				RefreshFilter();
//			}
//
//			if (m_Filter.length() > 0)
//			{
//				DrawItems(m_pFilteredAssets);
//			}
//			else
//			{
//				DrawItems(m_pPossibleAssets);
//			}
//
//			ImGui::EndPopup();
//		}
//	}
//
//	AssetPickerPopup::AssetPickerPopup() : m_FilterBuffer("")
//	{
//	}
//
//	AssetPickerPopup::~AssetPickerPopup()
//	{
//	}
//
//	void AssetPickerPopup::RefreshFilter()
//	{
//		m_pFilteredAssets.clear();
//		if (m_Filter == "") return;
//	
//		size_t compCount = 0;
//		for (size_t i = 0; i < m_pPossibleAssets.size(); i++)
//		{
//			if (m_pPossibleAssets[i]->Name().find(m_Filter) == std::string::npos) continue;
//			m_pFilteredAssets.push_back(m_pPossibleAssets[i]);
//		}
//	}
//
//	void AssetPickerPopup::LoadAssets()
//	{
//		m_pPossibleAssets.clear();
//		Spartan::AssetDatabase::GetAllAssetsOfType(m_pAssetReference->GetAssetType(), m_pPossibleAssets);
//	}
//
//	void AssetPickerPopup::DrawItems(const std::vector<Content*>& items)
//	{
//		if (ImGui::MenuItem("Noone"))
//		{
//			AssetSelected(nullptr);
//		}
//
//		std::for_each(items.begin(), items.end(), [&](Content* item)
//		{
//			if (ImGui::MenuItem(item->Name().c_str()))
//			{
//				AssetSelected(item);
//			}
//		});
//	}
//
//	void AssetPickerPopup::AssetSelected(Content* pAsset)
//	{
//		m_pAssetReference->SetAsset(pAsset);
//		m_pPossibleAssets.clear();
//		m_pFilteredAssets.clear();
//		m_pAssetReference = nullptr;
//		ImGui::CloseCurrentPopup();
//	}
//}