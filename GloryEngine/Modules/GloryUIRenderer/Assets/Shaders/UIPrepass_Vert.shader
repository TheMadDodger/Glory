#type vert
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std430, binding = 2) buffer ObjectData
{
	mat4 model;
	mat4 view;
	mat4 proj;
	uvec4 ObjectID;
} Object;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outColor;

void main()
{
    gl_Position = Object.proj * Object.model * vec4(inPosition.xy, 0.0, 1.0);
    outTexCoord = inTexCoord;
	outColor = inColor;
} 