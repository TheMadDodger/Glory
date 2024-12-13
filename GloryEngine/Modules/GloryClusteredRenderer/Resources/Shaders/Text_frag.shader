#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std430, binding = 2) buffer ObjectData
{
	mat4 model;
	mat4 view;
	mat4 proj;
	uvec4 ObjectID;
} Object;

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out uvec4 outID;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec4 outNormal;

void main()
{
	float pixel = texture(texSampler, fragTexCoord).r;
	if(pixel == 0.0) discard;
	outColor = pixel * inColor;
	outNormal = vec4((inNormal.xyz + 1.0) * 0.5, 1.0);
	outID = Object.ObjectID;
}
