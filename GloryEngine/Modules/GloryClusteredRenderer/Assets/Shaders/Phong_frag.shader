#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "internal/ObjectData.glsl"

layout(binding = 1, std430) buffer PropertiesSSBO
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

layout(location = 0) in vec3 normal;
layout(location = 1) in vec4 inColor;
layout(location = 2) in flat uint inObjectIndex;

layout(location = 0) out uvec4 outID;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec4 outNormal;
layout(location = 5) out vec4 outData;

void main()
{
	PerObjectData objectData = PerObjectDatas.Datas[inObjectIndex];
	Props props = PropertyData.PropertyDatas[objectData.MaterialIndex];

	outColor = inColor * props.Color;
	outNormal = vec4((normalize(normal) + 1.0) * 0.5, 1.0);
	outID = objectData.ObjectID;
	outData.g = props.Shininess;
	outData.a = 1.0;
}
