#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std430, binding = 2) buffer ObjectData
{
	mat4 model;
	mat4 view;
	mat4 proj;
	uint ObjectID;
} Object;

layout(binding = 0) uniform sampler2D texSampler;
layout(binding = 1) uniform sampler2D normalSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in mat3 TBN;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out uint outID;

void main()
{
	vec3 normal = texture(normalSampler, fragTexCoord).xyz * 2.0 - 1.0;
	normal = normalize(TBN * normal);
	outColor = texture(texSampler, fragTexCoord);
	outNormal = vec4((normal + 1.0) * 0.5, 1.0);
	outID = Object.ObjectID;
}
