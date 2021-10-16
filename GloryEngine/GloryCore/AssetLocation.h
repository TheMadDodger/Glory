#pragma once
#include <string>
#include <sstream>

namespace Glory
{
	struct AssetLocation
	{
		AssetLocation();
		AssetLocation(const std::string& path, size_t index);
		AssetLocation(const std::string& fullPath);

		std::string m_Path;
		size_t m_Index;
		bool m_IsSubAsset;
	};
}
