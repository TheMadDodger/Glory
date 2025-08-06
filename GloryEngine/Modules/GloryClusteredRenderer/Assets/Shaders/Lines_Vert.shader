#type vert
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std430, binding = 2) readonly buffer ObjectData
{
	mat4 model;
	mat4 view;
	mat4 proj;
	uint ObjectID;
} Object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main()
{
	gl_Position = Object.proj * Object.view * vec4(inPosition, 1.0);
	outColor = inColor;
}
