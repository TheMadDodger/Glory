#include "AssetLocation.h"

namespace Glory
{
	AssetLocation::AssetLocation()
		: Path(""), Index(0), IsSubAsset(false)
	{
	}

	AssetLocation::AssetLocation(const std::string& path, size_t index)
		: Path(path), Index(index), IsSubAsset(true)
	{
	}

	AssetLocation::AssetLocation(const std::string& fullPath)
	{
		int seperatorIndex = fullPath.find(':');
		if (seperatorIndex == std::string::npos)
		{
			Path = fullPath;
			Index = 0;
			IsSubAsset = false;
			return;
		}

		IsSubAsset = true;
		Path = fullPath.substr(0, seperatorIndex);
		std::string indexString = fullPath.substr(seperatorIndex + 1);
		std::istringstream reader(indexString);
		reader >> Index;
	}
}

namespace YAML
{
	Emitter& YAML::operator<<(Emitter& out, const Glory::AssetLocation& assetLoc)
	{
		out << YAML::BeginMap;

		out << YAML::Key << "Path";
		out << YAML::Value << assetLoc.Path;
		out << YAML::Key << "Index";
		out << YAML::Value << assetLoc.Index;
		out << YAML::Key << "IsSubAsset";
		out << YAML::Value << assetLoc.IsSubAsset;

		out << YAML::EndMap;

		return out;
	}
}
