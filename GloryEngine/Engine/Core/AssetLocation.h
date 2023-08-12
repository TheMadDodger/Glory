#pragma once
#include "ResourceMeta.h"
#include <string>
#include <sstream>

namespace Glory
{
	struct AssetLocation
	{
		AssetLocation();
		AssetLocation(const std::string& path, const std::string& subresourcePath = "", size_t index = 0);

		std::string Path;
		std::string SubresourcePath;
		size_t Index;
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
			node["SubresourcePath"] = assetLoc.SubresourcePath;
			node["Index"] = assetLoc.Index;
			return node;
		}

		static bool decode(const Node& node, Glory::AssetLocation& assetLoc)
		{
			if (!node.IsMap())
				return false;

			assetLoc.Path = node["Path"].as<std::string>();
			assetLoc.SubresourcePath = node["SubresourcePath"].as<std::string>();
			assetLoc.Index = node["Index"].as<size_t>();
			return true;
		}
	};
}
