#pragma once
#include "ResourceMeta.h"
#include <string>
#include <sstream>

namespace Glory
{
	struct AssetLocation
	{
		AssetLocation();
		AssetLocation(const std::string& path, size_t index, ResourceMeta* pMeta = nullptr);
		AssetLocation(const std::string& fullPath, ResourceMeta* pMeta = nullptr);

		std::string m_Path;
		size_t m_Index;
		bool m_IsSubAsset;
		ResourceMeta* m_pResourceMeta;
	};
}
