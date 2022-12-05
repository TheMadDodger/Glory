#version 430 core
layout(local_size_x = 16, local_size_y = 9, local_size_z = 4) in;

struct PointLight
{
    vec4 Position;
    vec4 Color;
    uint Enabled;
    float Intensity;
    float Range;
};

struct LightGridElement
{
    uint Offset;
    uint Count;
};

struct VolumeTileAABB
{
    vec4 MinPoint;
    vec4 MaxPoint;
};

layout(std430, binding = 1) buffer clusterAABB
{
    VolumeTileAABB Cluster[];
};

layout(std430, binding = 2) buffer screenToView
{
    mat4 ProjectionInverse;
    mat4 ViewInverse;
    uvec4 TileSizes;
    uvec2 ScreenDimensions;
    float Scale;
    float Bias;
};

layout(std430, binding = 3) buffer lightSSBO
{
    PointLight Lights[];
};

layout(std430, binding = 4) buffer lightIndexSSBO
{
    uint GlobalIndexCount;
    uint GlobalLightIndexList[];
};

layout(std430, binding = 5) buffer lightGridSSBO
{
    LightGridElement LightGrid[];
};

//Shared variables 
shared PointLight sharedLights[16 * 9 * 4];

uniform mat4 viewMatrix;

bool TestSphereAABB(uint light, uint tile);
float SQDistPointAABB(vec3 point, uint tile);

void main()
{
    GlobalIndexCount = 0;
    uint threadCount = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
    uint lightCount = Lights.length();
    uint numBatches = (lightCount + threadCount - 1) / threadCount;

    uint tileIndex = gl_LocalInvocationIndex + gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z * gl_WorkGroupID.z;

    uint visibleLightCount = 0;
    uint visibleLightIndices[100];

    for (uint batch = 0; batch < numBatches; ++batch)
    {
        uint lightIndex = batch * threadCount + gl_LocalInvocationIndex;

        //Prevent overflow by clamping to last light which is always null
        lightIndex = min(lightIndex, lightCount);

        //Populating shared light array
        sharedLights[gl_LocalInvocationIndex] = Lights[lightIndex];
        barrier();

        //Iterating within the current batch of lights
        for (uint light = 0; light < threadCount; ++light)
        {
            if (sharedLights[light].Enabled == 1)
            {
                if (TestSphereAABB(light, tileIndex))
                {
                    visibleLightIndices[visibleLightCount] = batch * threadCount + light;
                    visibleLightCount += 1;
                }
            }
        }
    }

    //We want all thread groups to have completed the light tests before continuing
    barrier();

    uint offset = atomicAdd(GlobalIndexCount, visibleLightCount);

    for (uint i = 0; i < visibleLightCount; ++i)
    {
        GlobalLightIndexList[offset + i] = visibleLightIndices[i];
    }

    LightGrid[tileIndex].Offset = offset;
    LightGrid[tileIndex].Count = visibleLightCount;
}

bool TestSphereAABB(uint light, uint tile)
{
    float radius = sharedLights[light].Range;
    vec3 center = vec3(viewMatrix * sharedLights[light].Position);
    float squaredDistance = SQDistPointAABB(center, tile);

    return squaredDistance <= (radius * radius);
}

float SQDistPointAABB(vec3 point, uint tile)
{
    float sqDist = 0.0;
    VolumeTileAABB currentCell = Cluster[tile];
    Cluster[tile].MaxPoint[3] = tile;
    for (int i = 0; i < 3; ++i)
    {
        float v = point[i];
        if (v < currentCell.MinPoint[i])
        {
            sqDist += (currentCell.MinPoint[i] - v) * (currentCell.MinPoint[i] - v);
        }
        if (v > currentCell.MaxPoint[i])
        {
            sqDist += (v - currentCell.MaxPoint[i]) * (v - currentCell.MaxPoint[i]);
        }
    }

    return sqDist;
}
