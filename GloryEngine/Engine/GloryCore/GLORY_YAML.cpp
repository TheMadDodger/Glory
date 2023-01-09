#include "GLORY_YAML.h"

namespace YAML
{
	Emitter& operator<<(Emitter& out, const Glory::LayerMask& mask)
	{
		out << mask.m_Mask;
		return out;
	}

	Emitter& operator<<(Emitter& out, const Glory::ShaderType& type)
	{
		out << SHADERTYPE_TOSTRING[type];
		return out;
	}

	Emitter& operator<<(Emitter& out, const Glory::LayerRef& layerRef)
	{
		out << layerRef.m_LayerIndex;
		return out;
	}
	Emitter& operator<<(Emitter& out, const Glory::SceneObjectRef& objectRef)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "SceneUUID";
		out << YAML::Value << objectRef.SceneUUID();
		out << YAML::Key << "ObjectUUID";
		out << YAML::Value << objectRef.ObjectUUID();
		out << YAML::EndMap;
		return out;
	}
}
