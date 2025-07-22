#type frag

#version 460
#extension GL_ARB_separate_shader_objects : enable
//#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_bindless_textures : enable

#include "internal/ObjectData.glsl"
#include "internal/Textured.glsl"

layout(std430, binding = 1) buffer PropertiesSSBO
{
	vec4 Color;
	float Shininess;
} Properties;

struct PerObjectData
{
	uvec4 ObjectID;
	mat4 World;
	uint MaterialIndex;
	uint padding1;
	uint padding2;
	uint padding3;
};

layout(std430, binding = 4) restrict readonly buffer PerObjectDataSSBO
{
	PerObjectData Datas[];
} PerObjectDatas;

struct Props
{
	vec4 Color;
	float Shininess;
};

layout(std430, binding = 6) restrict readonly buffer PropertyDataSSBO
{
	Props PropertyDatas[];
} PropertyData;

//struct TextureTypes
//{
//	sampler2D texSampler;
//	sampler2D normalSampler;
//	sampler2D shininessSampler;
//}

layout(binding = 0) uniform sampler2D texSampler[];
layout(binding = 1) uniform sampler2D normalSampler[];
layout(binding = 2) uniform sampler2D shininessSampler[];

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 inColor;
layout(location = 2) in flat uint inObjectIndex;
layout(location = 3) in mat3 TBN;

layout(location = 0) out uvec4 outID;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec4 outNormal;
layout(location = 5) out vec4 outData;

void main()
{
	PerObjectData objectData = PerObjectDatas.Datas[inObjectIndex];
	Props props = PropertyData.PropertyDatas[objectData.MaterialIndex];

	vec4 baseColor = TextureEnabled(0) ? texture(texSampler[objectData.MaterialIndex], fragTexCoord) : props.Color;
	if (baseColor.a == 0.0) discard;
	baseColor.a = 1.0;

	vec3 normal = TextureEnabled(1) ? (texture(normalSampler[objectData.MaterialIndex], fragTexCoord).xyz * 2.0 - 1.0) : TBN[2];
	float shininess = TextureEnabled(2) ? texture(shininessSampler[objectData.MaterialIndex], fragTexCoord).r : props.Shininess;
	normal = TextureEnabled(1) ? normalize(TBN * normal) : normal;

	outColor = baseColor * inColor;
	outNormal = vec4((normalize(normal) + 1.0) * 0.5, 1.0);
	outID = objectData.ObjectID;
	outData = vec4(1.0, shininess, 1.0, 1.0);
}
