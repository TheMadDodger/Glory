float LinearDepth(float depthSample, float zNear, float zFar)
{
    float depthRange = 2.0*depthSample - 1.0;
    float linear = 2.0*zNear*zFar/(zFar + zNear - depthRange*(zFar - zNear));
    return linear;
}

uint GetDepthSlice(float depth, float zNear, float zFar, float scale, float bias)
{
	return uint(max(log2(LinearDepth(depth, zNear, zFar))*scale + bias, 0.0));
}

vec4 ViewPosFromDepth(float depth, mat4 projectionInverse)
{
    float z = depth*2.0 - 1.0;

    vec4 clipSpacePosition = vec4(Coord*2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = projectionInverse*clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition;
}

vec3 WorldPosFromDepth(float depth, mat4 viewInverse, mat4 projectionInverse)
{
    vec4 worldSpacePosition = viewInverse*ViewPosFromDepth(depth, projectionInverse);
    return worldSpacePosition.xyz;
}

uint GetClusterIndex(vec3 pixelCoord, float zNear, float zFar, float scale, float bias, uvec4 tileSizes)
{
    uint clusterZVal = GetDepthSlice(pixelCoord.z, zNear, zFar, scale, bias);
    uvec3 clusters = uvec3(uvec2(pixelCoord.xy/tileSizes.w), clusterZVal);
    uint clusterIndex = clusters.x +
        tileSizes.x*clusters.y +
        (tileSizes.x*tileSizes.y)*clusters.z;
    return clusterIndex;
}