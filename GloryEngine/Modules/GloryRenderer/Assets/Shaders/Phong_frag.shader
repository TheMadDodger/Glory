#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Internal/RenderConstants.glsl"

struct Material
{
	vec4 Color;
	float Shininess;
};

#include "Internal/Material.glsl"

layout(location = 0) in vec3 normal;
layout(location = 1) in vec4 inColor;

layout(location = 0) out uvec4 outID;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec4 outNormal;
layout(location = 5) out vec4 outData;

void main()
{
	Material mat = GetMaterial();

	outColor = inColor*mat.Color;
	outNormal = vec4((normalize(normal) + 1.0)*0.5, 1.0);
	outID = Constants.ObjectID;
	outData.g = mat.Shininess;
	outData.a = 1.0;
}
