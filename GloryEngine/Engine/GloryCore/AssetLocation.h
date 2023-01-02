#pragma once
#include "ResourceMeta.h"
#include <string>
#include <sstream>

namespace Glory
{
	struct AssetLocation
	{
		AssetLocation();
		AssetLocation(const std::string& path, size_t index);
		AssetLocation(const std::string& fullPath);

		std::string Path;
		size_t Index;
		bool IsSubAsset;
	};
}

#include <yaml-cpp/yaml.h>
namespace YAML
{
	Emitter& operator<<(Emitter& out, const Glory::AssetLocation& assetLoc);

	template<>
	struct convert<Glory::AssetLocation>
	{
		static Node encode(const Glory::AssetLocation& assetLoc)
		{
			Node node;
			node = YAML::Node(YAML::NodeType::Map);
			node["Path"] = assetLoc.Path;
			node["Index"] = assetLoc.Index;
			node["IsSubAsset"] = assetLoc.IsSubAsset;
			return node;
		}

		static bool decode(const Node& node, Glory::AssetLocation& assetLoc)
		{
			if (!node.IsMap())
				return false;

			assetLoc.Path = node["Path"].as<std::string>();
			assetLoc.Index = node["Index"].as<size_t>();
			assetLoc.IsSubAsset = node["IsSubAsset"].as<bool>();
			return true;
		}
	};
}
