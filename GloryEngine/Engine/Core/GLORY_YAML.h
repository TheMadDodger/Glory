#pragma once
#include "LayerManager.h"
#include "LayerRef.h"
#include "SceneObjectRef.h"
#include "GraphicsEnums.h"

#include <yaml-cpp/yaml.h>
#include <string>
#include <algorithm>

namespace YAML
{
	Emitter& operator<<(Emitter& out, const Glory::LayerMask &mask);
	Emitter& operator<<(Emitter& out, const Glory::ShaderType &type);
	Emitter& operator<<(Emitter& out, const Glory::LayerRef &layerRef);
	Emitter& operator<<(Emitter& out, const Glory::SceneObjectRef& objectRef);

	template<>
	struct convert<Glory::LayerRef>
	{
		static Node encode(const Glory::LayerRef& layerRef)
		{
			Node node;
			node = layerRef.m_LayerIndex;
			return node;
		}

		static bool decode(const Node& node, Glory::LayerRef& layerRef)
		{
			if (!node.IsScalar())
				return false;

			layerRef = node.as<uint32_t>();
			return true;
		}
	};

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

	template<>
	struct convert<Glory::UUID>
	{
		static Node encode(const Glory::UUID& uuid)
		{
			Node node;
			node = (uint64_t)uuid;
			return node;
		}

		static bool decode(const Node& node, Glory::UUID& uuid)
		{
			if (!node.IsScalar())
				return false;

			uuid = node.as<uint64_t>();
			return true;
		}
	};

	template<>
	struct convert<Glory::SceneObjectRef>
	{
		static Node encode(const Glory::SceneObjectRef& objectRef)
		{
			Node node{YAML::NodeType::Map};
			node["SceneUUID"] = objectRef.SceneUUID();
			node["ObjectUUID"] = objectRef.ObjectUUID();
			return node;
		}

		static bool decode(const Node& node, Glory::SceneObjectRef& objectRef)
		{
			if (!node.IsMap())
				return false;

			objectRef = { node["SceneUUID"].as<uint64_t>(), node["ObjectUUID"].as<uint64_t>() };
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

	/** @todo: This is a duplicate */
	inline std::map<std::string, Glory::ShaderType> ShaderStringTypes = {
		{ "compute", Glory::ShaderType::ST_Compute },
		{ "comp", Glory::ShaderType::ST_Compute },
		{ "fragment", Glory::ShaderType::ST_Fragment },
		{ "frag", Glory::ShaderType::ST_Fragment },
		{ "geometry", Glory::ShaderType::ST_Geomtery },
		{ "geom", Glory::ShaderType::ST_Geomtery },
		{ "tesscontrol", Glory::ShaderType::ST_TessControl },
		{ "tc", Glory::ShaderType::ST_TessControl },
		{ "tesselationcontrol", Glory::ShaderType::ST_TessControl },
		{ "teseval", Glory::ShaderType::ST_TessEval },
		{ "te", Glory::ShaderType::ST_TessEval },
		{ "tesselationevaluation", Glory::ShaderType::ST_TessEval },
		{ "vertex", Glory::ShaderType::ST_Vertex },
		{ "vert", Glory::ShaderType::ST_Vertex },
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

			auto itor = ShaderStringTypes.find(typeString);
			if (itor == ShaderStringTypes.end())
			{
				type = Glory::ShaderType::ST_Unknown;
				return false;
			}

			type = itor->second;
			return true;
		}
	};
}