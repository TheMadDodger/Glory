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
		static GLORY_EDITOR_API bool ResourceThumbnailButton(const std::string& label, float buttonWidth, float popupStart, float popupWidth, uint32_t resourceType, UUID* value, bool includeSubAssets = true);

	private:
		AssetPicker() {}
		virtual ~AssetPicker() {}

		static void LoadAssets(uint32_t typeHash, bool includeSubAssets);
		static bool DrawPopup(UUID* value, uint32_t typeHash, bool includeSubAssets);
		static bool DrawItems(const std::vector<UUID>& items, UUID* value);

	private:
		static char m_SearchBuffer[200];
		static std::vector<UUID> m_SearchResultCache;
	};
}