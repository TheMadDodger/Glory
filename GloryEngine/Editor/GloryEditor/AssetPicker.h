#pragma once
#include <Resource.h>
#include <functional>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class AssetPicker
	{
	public:
		static GLORY_EDITOR_API bool ResourceDropdown(const std::string& label, uint32_t resourceType, UUID* value, bool includeSubAssets = true, const float borderPadding = 0.0f);
		static GLORY_EDITOR_API bool ResourceButton(const std::string& label, float buttonWidth, uint32_t resourceType, UUID* value, bool includeSubAssets = true);

	private:
		AssetPicker();
		virtual ~AssetPicker();

		static void RefreshFilter();
		static void LoadAssets(uint32_t typeHash, bool includeSubAssets);
		static bool DrawPopup(UUID* value, uint32_t typeHash, bool includeSubAssets);
		static bool DrawItems(const std::vector<UUID>& items, UUID* value);

	private:
		static char m_FilterBuffer[200];
		static std::string m_Filter;
		static std::vector<UUID> m_FilteredAssets;
		static std::vector<UUID> m_PossibleAssets;
	};
}