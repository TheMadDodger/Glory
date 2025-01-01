#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std430, binding = 1) buffer PropertiesSSBO
{
	vec4 Color;
} Properties;

layout(std430, binding = 2) buffer ObjectData
{
	mat4 model;
	mat4 view;
	mat4 proj;
	uvec4 ObjectID;
} Object;

layout(std430, binding = 3) buffer TexturesSSBO
{
	bool Color;
	bool Normal;
	bool Roughness;
} Textures;


layout(binding = 0) uniform sampler2D texSampler;
layout(binding = 1) uniform sampler2D normalSampler;
layout(binding = 2) uniform sampler2D ambientSampler;
layout(binding = 3) uniform sampler2D roughnessSampler;
layout(binding = 4) uniform sampler2D metalnessSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 inColor;
layout(location = 2) in mat3 TBN;

layout(location = 0) out uvec4 outID;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec4 outNormal;
layout(location = 4) out vec4 outPBR;

void main()
{
	vec3 normal = texture(normalSampler, fragTexCoord).xyz * 2.0 - 1.0;
	normal = normalize(TBN * normal);
	outColor = texture(texSampler, fragTexCoord) * inColor * Properties.Color;
	outNormal = vec4((normalize(normal) + 1.0) * 0.5, 1.0);
	outID = Object.ObjectID;

	float ambient = texture(ambientSampler, fragTexCoord).r;
	float roughness = texture(roughnessSampler, fragTexCoord).g;
	float metalic = texture(metalnessSampler, fragTexCoord).b;
	vec4 pbr;
	pbr.r = ambient;
	pbr.g = roughness;
	pbr.b = metalic;
	pbr.a = 1.0;
	outPBR = pbr;
}
