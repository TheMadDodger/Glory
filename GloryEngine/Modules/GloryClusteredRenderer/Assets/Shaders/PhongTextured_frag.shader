#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Internal/RenderConstants.glsl"
#include "internal/Textured1.glsl"

struct Material
{
	vec4 Color;
	float Shininess;
};

#include "Internal/Material.glsl"

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set = 2, binding = 1) uniform sampler2D normalSampler;
layout(set = 2, binding = 2) uniform sampler2D shininessSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 inColor;
layout(location = 2) in mat3 TBN;

layout(location = 0) out uvec4 outID;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec4 outNormal;
layout(location = 5) out vec4 outData;

void main()
{
	Material mat = GetMaterial();

	vec4 baseColor = TextureEnabled(0) ? texture(texSampler, fragTexCoord) : mat.Color;
	if (baseColor.a == 0.0) discard;
	baseColor.a = 1.0;

	vec3 normal = TextureEnabled(1) ? (texture(normalSampler, fragTexCoord).xyz * 2.0 - 1.0) : TBN[2];
	float shininess = TextureEnabled(2) ? texture(shininessSampler, fragTexCoord).r : mat.Shininess;
	normal = TextureEnabled(1) ? normalize(TBN * normal) : normal;

	outColor = baseColor * inColor;
	outNormal = vec4((normalize(normal) + 1.0) * 0.5, 1.0);
	outID = Constants.ObjectID;
	outData = vec4(1.0, shininess, 1.0, 1.0);
}
