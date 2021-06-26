#include "stdafx.h"
//#include "AssetReferencePropertyDrawer.h"
//#include "AssetPickerPopup.h"
//
//namespace Spartan::Editor
//{
//	bool AssetReferencePropertyDrawer::OnGUI(Serialization::SerializedProperty& prop) const
//	{
//		ImGui::Text((prop.m_Name + ": ").c_str());
//		ImGui::SameLine();
//		Content* pAsset = prop.m_pAssetReference->GetReferencedAsset();
//		std::string buttonText;
//		if (pAsset)
//		{
//			buttonText = pAsset->Name();
//		}
//		else
//		{
//			buttonText = "null";
//		}
//		ImGui::SameLine();
//		if (ImGui::Button(buttonText.c_str()))
//		{
//			// Open asset selection popup
//			AssetPickerPopup::Open(prop.m_pAssetReference);
//		}
//
//		if (prop.m_pAssetReference->IsDirty()) return true;
//		return false;
//	}
//}