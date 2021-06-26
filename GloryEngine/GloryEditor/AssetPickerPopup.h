//#pragma once
//#include <AssetReference.h>
//
//namespace Spartan::Editor
//{
//	class AssetPickerPopup
//	{
//	public:
//		static void Open(Spartan::Serialization::BaseAssetReference* pAssetReference);
//
//		void OnGUI();
//
//	private:
//		friend class EditorApp;
//		AssetPickerPopup();
//		virtual ~AssetPickerPopup();
//
//		void RefreshFilter();
//		void LoadAssets();
//		void DrawItems(const std::vector<Content*>& items);
//
//		void AssetSelected(Content* pAsset);
//
//	private:
//		static bool m_Open;
//		static Spartan::Serialization::BaseAssetReference* m_pAssetReference;
//		char m_FilterBuffer[200];
//		std::string m_Filter;
//		std::vector<Content*> m_pFilteredAssets;
//		std::vector<Content*> m_pPossibleAssets;
//	};
//}