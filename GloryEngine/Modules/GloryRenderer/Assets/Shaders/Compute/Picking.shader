#type compute
#version 430 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 16) in;

#ifdef PUSH_CONSTANTS
layout(push_constant) uniform PickingConstantsUBO
#else
layout(set = 0, std140, binding = 0) readonly uniform RenderConstantsUBO
#endif
{
    uint CameraIndex;
    uint NumPicks;
    ivec2 Picks[16];
} Constants;

#include "../Internal/Camera.glsl"
#include "../Internal/DepthHelpers.glsl"

struct PickResult
{
    vec4 ObjectID;
    vec4 Normal;
    vec4 Position;
};

layout(set = 1, std140, binding = 2) buffer PickingResultsSSBO
{
    uint NumPicks;
    PickResult Results[16];
};

layout (set = 2, binding = 0) uniform sampler2D ObjectID;
layout (set = 2, binding = 1) uniform sampler2D Normal;
layout (set = 2, binding = 2) uniform sampler2D Depth;

void main()
{
    CameraData camera = CurrentCamera();
    NumPicks = Constants.NumPicks;

    uint pickID = gl_LocalInvocationIndex;
    if (pickID >= Constants.NumPicks) return;

    ivec2 pickPos = Constants.Picks[pickID];
    vec2 coord = vec2(pickPos)/vec2(camera.Resolution);

    Results[pickID].ObjectID = texture(ObjectID, coord);
    Results[pickID].Normal = texture(Normal, coord);
    float depth = texture(Depth, coord).r;
    vec3 worldPosition = WorldPosFromDepth(depth, camera.ViewInverse, camera.ProjectionInverse, coord);
    Results[pickID].Position = vec4(worldPosition, 1.0);
}
