#pragma once
#include <Resource.h>
#include <functional>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class AssetPickerPopup
	{
	public:
		static GLORY_EDITOR_API void Open(size_t typeHash, UUID* pResource, bool* pHasChanged, bool includeSubAssets = false);
		static GLORY_EDITOR_API void Open(size_t typeHash, std::function<void(Resource*)> callback, bool includeSubAssets = false);
		GLORY_EDITOR_API void OnGUI();

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
		static bool* m_pHasChangedPointer;
		static size_t m_TypeHash;
		static bool m_IncludeSubAssets;
		char m_FilterBuffer[200];
		std::string m_Filter;
		std::vector<UUID> m_FilteredAssets;
		std::vector<UUID> m_PossibleAssets;
		static std::function<void(Resource*)> m_Callback;
	};
}