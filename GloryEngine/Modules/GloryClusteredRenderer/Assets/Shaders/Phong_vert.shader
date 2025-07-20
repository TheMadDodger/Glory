#type vert

#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shader_draw_parameters : enable

#include "internal/ObjectData.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoord;
layout(location = 5) in vec4 inColor;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec4 outColor;
layout(location = 2) out uint outObjectIndex;

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

layout(std430, binding = 5) restrict readonly buffer ObjectDataOffsetsSSBO
{
	uint Offsets[];
} ObjectDataOffsets;

void main()
{
	uint index = ObjectDataOffsets.Offsets[gl_DrawID];
	outObjectIndex = index + gl_InstanceIndex;
	PerObjectData objectData = PerObjectDatas.Datas[outObjectIndex];
	gl_Position = Object.proj * Object.view * objectData.World * vec4(inPosition, 1.0);
	normal = vec3(objectData.World * vec4(inNormal, 0.0));
	outColor = inColor;
}
