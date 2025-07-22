#version 430 core
layout(local_size_x = 1, local_size_y = 1) in;

struct BoundingBox
{
    vec4 Center;
    vec4 HalfExtends;
};

layout(std430, binding = 1) readonly buffer BoundingBoxSSBO
{
    BoundingBox BoundingBoxes[];
};

layout(std430, binding = 2) buffer ObjectData
{
	mat4 model;
	mat4 view;
	mat4 proj;
	uvec4 ObjectID;
} Object;

layout(std430, binding = 3) readonly buffer BoundingBoxIndicesSSBO
{
    uint BoundingBoxIndices[];
};

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

layout(std430, binding = 5) buffer CullingResultSSBO
{
	uint PassedObjectCount;
	uint PassedObjectIndex[];
};

//We will evaluate the whole screen in one compute shader
//so each thread is equivalent to a pixel
void main()
{
	PerObjectData objectData = PerObjectDatas.Datas[gl_WorkGroupID.x];
	uint boundingBoxIndex = BoundingBoxIndices[gl_WorkGroupID.x];
    BoundingBox box = BoundingBoxes[boundingBoxIndex];
    mat4 world = objectData.World;

	vec3 center = box.Center.xyz;
    vec3 points[8];
    points[0] = center + vec3(-box.HalfExtends.x, box.HalfExtends.y, -box.HalfExtends.z);
	points[1] = center + vec3(box.HalfExtends.x, box.HalfExtends.y, -box.HalfExtends.z);
	points[2] = center + vec3(box.HalfExtends.x, box.HalfExtends.y, box.HalfExtends.z);
	points[3] = center + vec3(-box.HalfExtends.x, box.HalfExtends.y, box.HalfExtends.z);
	points[4] = center + vec3(-box.HalfExtends.x, -box.HalfExtends.y, -box.HalfExtends.z);
	points[5] = center + vec3(box.HalfExtends.x, -box.HalfExtends.y, -box.HalfExtends.z);
	points[6] = center + vec3(box.HalfExtends.x, -box.HalfExtends.y, box.HalfExtends.z);
	points[7] = center + vec3(-box.HalfExtends.x, -box.HalfExtends.y, box.HalfExtends.z);

	for (int i = 0; i < 8; ++i)
	{
		vec4 point = Object.proj*Object.view*world*vec4(points[i], 1.0);
		if (!(point.x >= -1.0 && point.x <= 1.0 && point.y >= -1.0 &&
			point.y <= 1.0 && point.z >= -1.0 && point.z <= 1.0)) continue;

		uint index = atomicAdd(PassedObjectCount, 1);
		PassedObjectIndex[index] = gl_WorkGroupID.x;
		break;
	}
}
