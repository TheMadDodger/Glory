#include "AssetLocation.h"

namespace Glory
{
	AssetLocation::AssetLocation()
		: Path(""), SubresourcePath(""), Index(0)
	{
	}

	AssetLocation::AssetLocation(const std::string& path, const std::string& subresourcePath, size_t index)
		: Path(path), SubresourcePath(subresourcePath), Index(index)
	{
	}

	//AssetLocation::AssetLocation(const std::string& fullPath)
	//{
	//	int seperatorIndex = fullPath.find(':');
	//	if (seperatorIndex == std::string::npos)
	//	{
	//		Path = fullPath;
	//		Index = 0;
	//		IsSubAsset = false;
	//		return;
	//	}
	//
	//	IsSubAsset = true;
	//	Path = fullPath.substr(0, seperatorIndex);
	//	std::string indexString = fullPath.substr(seperatorIndex + 1);
	//	std::istringstream reader(indexString);
	//	reader >> Index;
	//}
}

namespace YAML
{
	Emitter& YAML::operator<<(Emitter& out, const Glory::AssetLocation& assetLoc)
	{
		out << YAML::BeginMap;

		out << YAML::Key << "Path";
		out << YAML::Value << assetLoc.Path;
		out << YAML::Key << "SubresourcePath";
		out << YAML::Value << assetLoc.SubresourcePath;
		out << YAML::Key << "Index";
		out << YAML::Value << assetLoc.Index;

		out << YAML::EndMap;

		return out;
	}
}
