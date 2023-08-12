#type vert

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std430, binding = 2) buffer ObjectData
{
	mat4 model;
	mat4 view;
	mat4 proj;
	uint ObjectID;
} Object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoord;
layout(location = 5) in vec4 inColor;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 outColor;
layout(location = 2) out mat3 TBN;

void main()
{
	gl_Position = Object.proj * Object.view * Object.model * vec4(inPosition, 1.0);
	fragTexCoord = inTexCoord;

	vec3 T = normalize(vec3(Object.model * vec4(inTangent, 0.0)));
	vec3 B = normalize(vec3(Object.model * vec4(inBitangent, 0.0)));
	vec3 N = normalize(vec3(Object.model * vec4(inNormal, 0.0)));
	TBN = mat3(T, B, N);

	outColor = inColor;
}
