#pragma once

#include <MonoComponents.h>
#include <yaml-cpp/yaml.h>

namespace YAML
{
	Emitter& operator<<(Emitter& out, const Glory::ScriptTypeReference& ref);

	template<>
	struct convert<Glory::ScriptTypeReference>
	{
		static Node encode(const Glory::ScriptTypeReference& ref)
		{
			Node node;
			node = ref.m_Hash;
			return node;
		}

		static bool decode(const Node& node, Glory::ScriptTypeReference& ref)
		{
			if (!node.IsScalar())
				return false;

			ref.m_Hash = node.as<uint32_t>();
			return true;
		}
	};
}