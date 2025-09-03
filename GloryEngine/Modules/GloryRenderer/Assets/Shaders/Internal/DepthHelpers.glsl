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

float LinearDepth(float depthSample)
{
    float depthRange = 2.0 * depthSample - 1.0;
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    return linear;
}

uint GetDepthSlice(float depth)
{
	return uint(max(log2(LinearDepth(depth)) * Scale + Bias, 0.0));
}

vec4 ViewPosFromDepth(float depth)
{
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(Coord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = ProjectionInverse * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition;
}

vec3 WorldPosFromDepth(float depth)
{
    vec4 worldSpacePosition = ViewInverse * ViewPosFromDepth(depth);
    return worldSpacePosition.xyz;
}

uint GetClusterIndex(vec3 pixelCoord)
{
    uint clusterZVal = GetDepthSlice(pixelCoord.z);

    uvec3 clusters = uvec3(uvec2(pixelCoord.xy / TileSizes.w), clusterZVal);
    uint clusterIndex = clusters.x +
        TileSizes.x * clusters.y +
        (TileSizes.x * TileSizes.y) * clusters.z;
    return clusterIndex;
}
