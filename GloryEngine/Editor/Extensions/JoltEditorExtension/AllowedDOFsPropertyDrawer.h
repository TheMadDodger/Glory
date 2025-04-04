#pragma once
#include <PropertyDrawer.h>

#include <Physics.h>
#include <yaml-cpp/yaml.h>

namespace YAML
{
	Emitter& operator<<(Emitter& out, const Glory::AllowedDOFFlag& value);

	template<>
	struct convert<Glory::AllowedDOFFlag>
	{
		static Node encode(const Glory::AllowedDOFFlag& value)
		{
			Node node;
			node = uint16_t(value);
			return node;
		}

		static bool decode(const Node& node, Glory::AllowedDOFFlag& value)
		{
			if (!node.IsScalar())
				return false;

			value = Glory::AllowedDOFFlag(node.as<uint16_t>());
			return true;
		}
	};
}