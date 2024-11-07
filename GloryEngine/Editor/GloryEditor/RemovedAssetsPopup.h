#pragma once
#include "GloryEditor.h"

#include <UUID.h>

#include <string>
#include <vector>

namespace Glory
{
	struct AssetLocation;
}

namespace Glory::Editor
{
	class RemovedAssetsPopup
	{
	public:
		GLORY_EDITOR_API static void AddRemovedAssets(std::vector<AssetLocation>&& assets);

	private:
		static void Draw();

	private:
		friend class MainEditor;

	private:
		static std::vector<AssetLocation> m_RemovedAssets;
		RemovedAssetsPopup() = delete;
	};
}
