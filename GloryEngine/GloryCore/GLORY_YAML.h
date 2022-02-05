#pragma once
#include <yaml-cpp/yaml.h>
#include "LayerManager.h"

namespace YAML
{
	Emitter& operator<<(Emitter& out, const Glory::LayerMask &mask);

	template<>
	struct convert<Glory::LayerMask>
	{
		static Node encode(const Glory::LayerMask& mask)
		{
			Node node;
			node = mask.m_Mask;
		}

		static bool decode(const Node& node, Glory::LayerMask& mask)
		{
			if (!node.IsScalar())
				return false;

			mask = node.as<uint64_t>();
			return true;
		}
	};
}