#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1, std430) buffer PropertiesSSBO
{
	vec4 Color;
} Properties;

layout(std430, binding = 2) buffer ObjectData
{
	mat4 model;
	mat4 view;
	mat4 proj;
	uint ObjectID;
} Object;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out uint outID;

void main()
{
	outColor = inColor * Properties.Color;
	outNormal = vec4((normal + 1.0) * 0.5, 1.0);
	outID = Object.ObjectID;
}
