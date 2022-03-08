#pragma once
#include <Resource.h>
#include <functional>

namespace Glory::Editor
{
	class AssetPickerPopup
	{
	public:
		static void Open(size_t typeHash, Resource** pResource);
		static void Open(size_t typeHash, std::function<void(Resource*)> callback);
		void OnGUI();

	private:
		AssetPickerPopup();
		virtual ~AssetPickerPopup();

		void RefreshFilter();
		void LoadAssets();
		void DrawItems(const std::vector<UUID>& items);

		void AssetSelected(Resource* pResource);

	private:
		friend class MainEditor;
		static bool m_Open;
		static Resource** m_pResourcePointer;
		static size_t m_TypeHash;
		char m_FilterBuffer[200];
		std::string m_Filter;
		std::vector<UUID> m_FilteredAssets;
		std::vector<UUID> m_PossibleAssets;
		static std::function<void(Resource*)> m_Callback;
	};
}