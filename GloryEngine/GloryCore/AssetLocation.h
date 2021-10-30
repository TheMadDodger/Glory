#pragma once
#include "ResourceMeta.h"
#include <string>
#include <sstream>

namespace Glory
{
	struct AssetLocation
	{
		AssetLocation();
		AssetLocation(const std::string& path, size_t index, const ResourceMeta& pMeta);
		AssetLocation(const std::string& fullPath, const ResourceMeta& pMeta);

		std::string m_Path;
		size_t m_Index;
		bool m_IsSubAsset;
	};
}
