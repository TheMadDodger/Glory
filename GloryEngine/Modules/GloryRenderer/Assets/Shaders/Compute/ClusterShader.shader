#type compute
#version 430 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#include "../Internal/Clusters.glsl"
#include "../Internal/Camera.glsl"

/* Function prototypes */
vec3 LineIntersectionWithZPlane(vec3 startPoint, vec3 endPoint, float zDistance);
vec3 ScreenToView(vec2 screenCoord, mat4 inverseProjection, vec2 screenDimensions);

/*
 * Source: https://github.com/DaveH355/clustered-shading
 * context: glViewport is referred to as the "screen"
 * clusters are built based on a 2d screen-space grid and depth slices.
 * Later when shading, it is easy to figure what cluster a fragment is in based on
 * gl_FragCoord.xy and the fragment's z depth from camera
 */
void main()
{
    CameraData camera = CurrentCamera();

    uint tileIndex = gl_WorkGroupID.x + (gl_WorkGroupID.y*Constants.GridSize.x) +
        (gl_WorkGroupID.z*Constants.GridSize.x*Constants.GridSize.y);
    vec2 tileSize = camera.Resolution/Constants.GridSize.xy;

    /* Tile in screen-space */
    vec2 minTile_screenspace = gl_WorkGroupID.xy*tileSize;
    vec2 maxTile_screenspace = (gl_WorkGroupID.xy + 1)*tileSize;

    /* Convert tile to view space sitting on the near plane */
    vec3 minTile = ScreenToView(minTile_screenspace, camera.ProjectionInverse, camera.Resolution);
    vec3 maxTile = ScreenToView(maxTile_screenspace, camera.ProjectionInverse, camera.Resolution);

    float planeNear = camera.zNear*pow(camera.zFar/camera.zNear, gl_WorkGroupID.z/float(Constants.GridSize.z));
    float planeFar = camera.zNear*pow(camera.zFar/camera.zNear, (gl_WorkGroupID.z + 1)/float(Constants.GridSize.z));

    /*
     * The line goes from the eye position in view space (0, 0, 0)
     * through the min/max points of a tile to intersect with a given cluster's near-far planes
     */
    vec3 minPointNear = LineIntersectionWithZPlane(vec3(0, 0, 0), minTile, planeNear);
    vec3 minPointFar = LineIntersectionWithZPlane(vec3(0, 0, 0), minTile, planeFar);
    vec3 maxPointNear = LineIntersectionWithZPlane(vec3(0, 0, 0), maxTile, planeNear);
    vec3 maxPointFar = LineIntersectionWithZPlane(vec3(0, 0, 0), maxTile, planeFar);

    Cluster[tileIndex].MinPoint = vec4(min(minPointNear, minPointFar), 0.0);
    Cluster[tileIndex].MaxPoint = vec4(max(maxPointNear, maxPointFar), 0.0);
}

/*
 * Returns the intersection point of an infinite line and a
 * plane perpendicular to the Z-axis
 */
vec3 LineIntersectionWithZPlane(vec3 startPoint, vec3 endPoint, float zDistance)
{
    vec3 direction = endPoint - startPoint;
    vec3 normal = vec3(0.0, 0.0, -1.0); /* Plane normal */

    /* Skip check if the line is parallel to the plane. */

    float t = (zDistance - dot(normal, startPoint))/dot(normal, direction);
    return startPoint + t*direction; /* The parametric form of the line equation */
}

vec3 ScreenToView(vec2 screenCoord, mat4 inverseProjection, vec2 screenDimensions)
{
    /*
     * Normalize screenCoord to [-1, 1] and
     * set the NDC depth of the coordinate to be on the near plane. This is -1 by
     * default in OpenGL
     */
    vec4 ndc = vec4(screenCoord/screenDimensions*2.0 - 1.0, -1.0, 1.0);

    vec4 viewCoord = inverseProjection*ndc;
    viewCoord /= viewCoord.w;
    return viewCoord.xyz;
}
