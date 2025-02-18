#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

#define FEATURE_TEXTURED

layout(std430, binding = 2) buffer ObjectData
{
	mat4 model;
	mat4 view;
	mat4 proj;
	uvec4 ObjectID;
} Object;

#ifdef WITH_TEXTURED
layout(binding = 0) uniform sampler2D texSampler;
layout(binding = 1) uniform sampler2D normalSampler;
layout(binding = 2) uniform sampler2D ambientSampler;
layout(binding = 3) uniform sampler2D roughnessSampler;
layout(binding = 4) uniform sampler2D metalnessSampler;
#else
layout(std430, binding = 1) buffer PropertiesSSBO
{
	vec4 Color;
	float AO;
	float Roughness;
	float Metalness;

} Properties;
#endif

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 inColor;
#ifdef WITH_TEXTURED
layout(location = 2) in mat3 TBN;
#else
layout(location = 2) in vec3 inNormal;
#endif

layout(location = 0) out uvec4 outID;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec4 outNormal;
layout(location = 4) out vec4 outPBR;

void main()
{
#ifdef WITH_TEXTURED
	outColor = texture(texSampler, fragTexCoord) * inColor;
	vec3 normal = texture(normalSampler, fragTexCoord).xyz * 2.0 - 1.0;
	normal = normalize(TBN * normal);
	float ambient = texture(ambientSampler, fragTexCoord).r;
	float roughness = texture(roughnessSampler, fragTexCoord).g;
	float metalic = texture(metalnessSampler, fragTexCoord).b;
#else
	outColor = inColor * Properties.Color;
	vec3 normal = inNormal;
	float ambient = Properties.AO;
	float roughness = Properties.Roughness;
	float metalic = Properties.Metalness;
#endif
	outNormal = vec4((normalize(normal) + 1.0) * 0.5, 1.0);
	outID = Object.ObjectID;

	vec4 pbr;
	pbr.r = ambient;
	pbr.g = roughness;
	pbr.b = metalic;
	pbr.a = 1.0;
	outPBR = pbr;
}
