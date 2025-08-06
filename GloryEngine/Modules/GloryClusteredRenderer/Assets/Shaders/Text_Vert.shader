#type vert
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std430, binding = 2) readonly buffer ObjectData
{
	mat4 model;
	mat4 view;
	mat4 proj;
	uvec4 ObjectID;
} Object;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec2 fragTexCoord;

void main()
{
	gl_Position = Object.proj * Object.view * Object.model * vec4(inPosition, 0.0, 1.0);
	outNormal = Object.model * vec4(0.0, 0.0, 1.0, 0.0);
	fragTexCoord = inTexCoord;
	outColor = vec4(inColor, 1.0);
}
