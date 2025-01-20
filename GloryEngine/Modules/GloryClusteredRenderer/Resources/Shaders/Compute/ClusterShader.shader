#version 430 core
layout(local_size_x = 1, local_size_y = 1) in;

//ssbo initialization
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
    float zNear;
	float zFar;
};

//Function prototypes
vec4 ClipToView(vec4 clip);
vec4 Screen2View(vec4 screen);
vec3 LineIntersectionToZPlane(vec3 A, vec3 B, float zDistance);

void main()
{
    //Eye position is zero in view space
    const vec3 eyePos = vec3(0.0);

    //Per Tile variables
    uint tileSizePx = TileSizes.w;
    uint tileIndex = gl_WorkGroupID.x +
        gl_WorkGroupID.y * gl_NumWorkGroups.x +
        gl_WorkGroupID.z * (gl_NumWorkGroups.x * gl_NumWorkGroups.y);

    //Calculating the min and max point in screen space
    vec4 maxPoint_sS = vec4(vec2(gl_WorkGroupID.x + 1, gl_WorkGroupID.y + 1) * tileSizePx, -1.0, 1.0); // Top Right
    vec4 minPoint_sS = vec4(gl_WorkGroupID.xy * tileSizePx, -1.0, 1.0); // Bottom left

    //Pass min and max to view space
    vec3 maxPoint_vS = Screen2View(maxPoint_sS).xyz;
    vec3 minPoint_vS = Screen2View(minPoint_sS).xyz;

    //Near and far values of the cluster in view space
    float tileNear = -zNear * pow(zFar / zNear, gl_WorkGroupID.z / float(gl_NumWorkGroups.z));
    float tileFar = -zNear * pow(zFar / zNear, (gl_WorkGroupID.z + 1) / float(gl_NumWorkGroups.z));

    //Finding the 4 intersection points made from the maxPoint to the cluster near/far plane
    vec3 minPointNear = LineIntersectionToZPlane(eyePos, minPoint_vS, tileNear);
    vec3 minPointFar = LineIntersectionToZPlane(eyePos, minPoint_vS, tileFar);
    vec3 maxPointNear = LineIntersectionToZPlane(eyePos, maxPoint_vS, tileNear);
    vec3 maxPointFar = LineIntersectionToZPlane(eyePos, maxPoint_vS, tileFar);

    vec3 minPointAABB = min(min(minPointNear, minPointFar), min(maxPointNear, maxPointFar));
    vec3 maxPointAABB = max(max(minPointNear, minPointFar), max(maxPointNear, maxPointFar));

    //Getting the 
    Cluster[tileIndex].MinPoint = vec4(minPointAABB, 0.0);
    Cluster[tileIndex].MaxPoint = vec4(maxPointAABB, 0.0);
}

//Creates a line from the eye to the screenpoint, then finds its intersection
//With a z oriented plane located at the given distance to the origin
vec3 LineIntersectionToZPlane(vec3 A, vec3 B, float zDistance)
{
    //Because this is a Z based normal this is fixed
    vec3 normal = vec3(0.0, 0.0, 1.0);

    vec3 ab = B - A;

    //Computing the intersection length for the line and the plane
    float t = (zDistance - dot(normal, A)) / dot(normal, ab);

    //Computing the actual xyz position of the point along the line
    vec3 result = A + t * ab;

    return result;
}

vec4 ClipToView(vec4 clip)
{
    //View space transform
    vec4 view = ProjectionInverse * clip;

    //Perspective projection
    view = view / view.w;

    return view;
}

vec4 Screen2View(vec4 screen)
{
    //Convert to NDC
    vec2 texCoord = screen.xy / ScreenDimensions.xy;

    //Convert to clipSpace
    // vec4 clip = vec4(vec2(texCoord.x, 1.0 - texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
    vec4 clip = vec4(vec2(texCoord.x, texCoord.y) * 2.0 - 1.0, screen.z, screen.w);
    //Not sure which of the two it is just yet

    return ClipToView(clip);
}
