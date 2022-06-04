#pragma once
#include <Resource.h>
#include <functional>

namespace Glory::Editor
{
	class AssetPickerPopup
	{
	public:
		static void Open(size_t typeHash, UUID* pResource, bool includeSubAssets = false);
		static void Open(size_t typeHash, std::function<void(Resource*)> callback, bool includeSubAssets = false);
		void OnGUI();

	private:
		AssetPickerPopup();
		virtual ~AssetPickerPopup();

		void RefreshFilter();
		void LoadAssets();
		void DrawItems(const std::vector<UUID>& items);

		void AssetSelected(UUID uuid);

	private:
		friend class MainEditor;
		static bool m_Open;
		static UUID* m_pUUIDPointer;
		static size_t m_TypeHash;
		static bool m_IncludeSubAssets;
		char m_FilterBuffer[200];
		std::string m_Filter;
		std::vector<UUID> m_FilteredAssets;
		std::vector<UUID> m_PossibleAssets;
		static std::function<void(Resource*)> m_Callback;
	};
}