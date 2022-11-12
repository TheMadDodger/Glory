#pragma once
#include <yaml-cpp/yaml.h>
#include <string>
#include <algorithm>
#include "LayerManager.h"
#include "ShaderSourceLoaderModule.h"

namespace YAML
{
	Emitter& operator<<(Emitter& out, const Glory::LayerMask &mask);
	Emitter& operator<<(Emitter& out, const Glory::ShaderType &type);

	template<>
	struct convert<Glory::LayerMask>
	{
		static Node encode(const Glory::LayerMask& mask)
		{
			Node node;
			node = mask.m_Mask;
			return node;
		}

		static bool decode(const Node& node, Glory::LayerMask& mask)
		{
			if (!node.IsScalar())
				return false;

			mask = node.as<uint64_t>();
			return true;
		}
	};

	inline std::map<Glory::ShaderType, std::string> SHADERTYPE_TOSTRING = {
		{ Glory::ShaderType::ST_Compute, "Comp" },
		{ Glory::ShaderType::ST_Fragment, "Frag" },
		{ Glory::ShaderType::ST_Vertex, "Vert" },
		{ Glory::ShaderType::ST_Geomtery, "Geom" },
		{ Glory::ShaderType::ST_TessControl, "TessControl" },
		{ Glory::ShaderType::ST_TessEval, "TessEval" },
	};

	inline std::map<Glory::ShaderType, std::string> SHADERTYPE_TOFULLSTRING = {
		{ Glory::ShaderType::ST_Compute, "Compute" },
		{ Glory::ShaderType::ST_Fragment, "Fragment" },
		{ Glory::ShaderType::ST_Vertex, "Vertex" },
		{ Glory::ShaderType::ST_Geomtery, "Geometry" },
		{ Glory::ShaderType::ST_TessControl, "TessControl" },
		{ Glory::ShaderType::ST_TessEval, "TessEval" },
	};

	template<>
	struct convert<Glory::ShaderType>
	{
		static Node encode(const Glory::ShaderType& type)
		{
			Node node;
			node = SHADERTYPE_TOSTRING[type];
			return node;
		}

		static bool decode(const Node& node, Glory::ShaderType& type)
		{
			if (!node.IsScalar())
				return false;

			std::string typeString = node.as<std::string>();

			std::transform(typeString.begin(), typeString.end(), typeString.begin(),
				[](unsigned char c) { return std::tolower(c); });

			return Glory::ShaderSourceLoaderModule::GetShaderTypeFromString(typeString, type);
		}
	};
}