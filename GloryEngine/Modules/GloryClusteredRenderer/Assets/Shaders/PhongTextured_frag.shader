#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "internal/ObjectData.glsl"

layout(std430, binding = 1) buffer PropertiesSSBO
{
	vec4 Color;
} Properties;

layout(binding = 0) uniform sampler2D texSampler;
layout(binding = 1) uniform sampler2D normalSampler;
layout(binding = 2) uniform sampler2D shininessSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 inColor;
layout(location = 2) in mat3 TBN;

layout(location = 0) out uvec4 outID;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec4 outNormal;
layout(location = 5) out vec4 outData;

void main()
{
	vec3 normal = texture(normalSampler, fragTexCoord).xyz * 2.0 - 1.0;
	float shininess = texture(shininessSampler, fragTexCoord).r;
	normal = normalize(TBN * normal);
	outColor = texture(texSampler, fragTexCoord) * inColor * Properties.Color;
	outNormal = vec4((normalize(normal) + 1.0) * 0.5, 1.0);
	outID = Object.ObjectID;
	outData = vec4(1.0, shininess, 1.0, 1.0);
}
