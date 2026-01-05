#type compute
#version 430 core
layout(local_size_x = 16, local_size_y = 9, local_size_z = 4) in;

#include "../Internal/Clusters.glsl"
#include "../Internal/Camera.glsl"
#include "../Internal/Light.glsl"

layout(set = 4, std430, binding = 7) buffer LightDistanceSSBO
{
    uint LightDepthSlices[];
};

/* Shared variables */
shared CompactLightData sharedLights[16*9*4];
shared uint sharedLightIndices[16*9*4];

bool TestSphereAABB(uint light, uint tile, mat4 view);
bool TestConeAABB(uint light, uint tile, mat4 view);
float SQDistPointAABB(vec3 point, uint tile);

const float PI = 3.14159265359;

void main()
{
    CameraData camera = CurrentCamera();

    GlobalIndexCount = 0;
    uint threadCount = gl_WorkGroupSize.x*gl_WorkGroupSize.y*gl_WorkGroupSize.z;
    uint lightCount = Lights.length();
    uint numBatches = (lightCount + threadCount - 1)/threadCount;

    uint tileIndex = gl_LocalInvocationIndex +
        gl_WorkGroupSize.x*gl_WorkGroupSize.y*gl_WorkGroupSize.z*gl_WorkGroupID.z;

    uint visibleLightCount = 0;
    uint visibleLightIndices[100];

    for (uint batch = 0; batch < numBatches; ++batch)
    {
        uint lightIndex = batch*threadCount + gl_LocalInvocationIndex;

        /* Prevent overflow by clamping to last light which is always null */
        lightIndex = min(lightIndex, Lights.length());

        /* Populating shared light array */
        sharedLights[gl_LocalInvocationIndex].Position = Lights[lightIndex].Position;
        sharedLights[gl_LocalInvocationIndex].Direction = Lights[lightIndex].Direction;
        sharedLights[gl_LocalInvocationIndex].Data = Lights[lightIndex].Data;
        sharedLights[gl_LocalInvocationIndex].Type = Lights[lightIndex].Type;
        sharedLightIndices[gl_LocalInvocationIndex] = lightIndex;
        barrier();

        /* Iterating within the current batch of lights */
        for (uint light = 0; light < threadCount; ++light)
        {
            if(sharedLightIndices[light] < Constants.LightCount)
            {
                if (sharedLights[light].Type == Point && TestSphereAABB(light, tileIndex, camera.View))
                {
                    visibleLightIndices[visibleLightCount] = batch*threadCount + light;
                    visibleLightCount += 1;
                }
                if (sharedLights[light].Type == Sun)
                {
                    visibleLightIndices[visibleLightCount] = batch*threadCount + light;
                    visibleLightCount += 1;
                }
                if (sharedLights[light].Type == Spot && TestConeAABB(light, tileIndex, camera.View))
                {
                    visibleLightIndices[visibleLightCount] = batch*threadCount + light;
                    visibleLightCount += 1;
                }
            }
        }
    }

    /* We want all thread groups to have completed the light tests before continuing */
    barrier();

    uint offset = atomicAdd(GlobalIndexCount, visibleLightCount);

    uint depthSlice = tileIndex/(Constants.GridSize.x*Constants.GridSize.y);
    for (uint i = 0; i < visibleLightCount; ++i)
    {
        uint lightIndex = visibleLightIndices[i];
        GlobalLightIndexList[offset + i] = lightIndex;
        atomicMin(LightDepthSlices[lightIndex], depthSlice);
    }

    LightGrid[tileIndex].Offset = offset;
    LightGrid[tileIndex].Count = visibleLightCount;
}

bool TestSphereAABB(uint light, uint tile, mat4 view)
{
    float radius = sharedLights[light].Data.y;
    vec3 center = vec3(view*vec4(sharedLights[light].Position.xyz, 1.0));
    float squaredDistance = SQDistPointAABB(center, tile);
    return squaredDistance <= (radius*radius);
}

vec4 ConeBoundingSphere(vec3 origin, vec3 forward, float size, float angle)
{
    vec4 boundingSphere;
    if (angle > PI/4.0)
    {
        boundingSphere.xyz = origin + cos(angle)*size*forward;
        boundingSphere.w = sin(angle)*size;
    }
    else
    {
        boundingSphere.xyz = origin + size/(2.0*cos(angle))*forward;
        boundingSphere.w = size/(2.0*cos(angle));
    }
    return boundingSphere;
}

bool TestConeAABB(uint light, uint tile, mat4 view)
{
    float outerAngle = sharedLights[light].Data.y;
    float range = sharedLights[light].Data.z;
    vec3 start = sharedLights[light].Position.xyz;
    vec3 direction = sharedLights[light].Direction.xyz;
    vec4 boundingSphere = ConeBoundingSphere(start, -direction, range, outerAngle*PI/180.0);
    vec3 center = vec3(view*vec4(boundingSphere.xyz, 1.0));
    float endSphereRadius = range*tan(outerAngle*PI/180.0/2.0);
    float squaredDistance = SQDistPointAABB(center, tile);
    center = vec3(view*vec4(start - direction*range, 1.0));
    float squaredDistance2 = SQDistPointAABB(center, tile);
    float radius = boundingSphere.w;
    return squaredDistance <= (radius*radius) || squaredDistance2 <= (endSphereRadius*endSphereRadius);
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
            sqDist += (currentCell.MinPoint[i] - v)*(currentCell.MinPoint[i] - v);
        }
        if (v > currentCell.MaxPoint[i])
        {
            sqDist += (v - currentCell.MaxPoint[i])*(v - currentCell.MaxPoint[i]);
        }
    }

    return sqDist;
}
