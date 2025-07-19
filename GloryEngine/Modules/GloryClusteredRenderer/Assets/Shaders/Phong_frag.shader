#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "internal/ObjectData.glsl"

layout(binding = 1, std430) buffer PropertiesSSBO
{
	vec4 Color;
	float Shininess;
} Properties;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec4 inColor;
layout(location = 2) flat in uvec4 inObjectID;

layout(location = 0) out uvec4 outID;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec4 outNormal;
layout(location = 5) out vec4 outData;

void main()
{
	outColor = inColor * Properties.Color;
	outNormal = vec4((normalize(normal) + 1.0) * 0.5, 1.0);
	outID = inObjectID;
	outData.g = Properties.Shininess;
	outData.a = 1.0;
}
