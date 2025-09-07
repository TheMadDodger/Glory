#ifdef PUSH_CONSTANTS
layout(push_constant) uniform RenderConstantsUBO
#else
layout(set = 0, std140, binding = 0) readonly uniform RenderConstantsUBO
#endif
{
    uvec4 TileSizes;
    uint LightCount;
	uint CameraIndex;
    float Scale;
    float Bias;
} Constants;

struct VolumeTileAABB
{
    vec4 MinPoint;
    vec4 MaxPoint;
};

layout(set = 1, std430, binding = 2) buffer clusterAABB
{
    VolumeTileAABB Cluster[];
};
