#version 430 core
layout(local_size_x = 1, local_size_y = 1) in;

//Shared between all clusters
uniform float zNear;
uniform float zFar;
uniform uint tileSizeInPx; // How many pixels a rectangular cluster takes in x and y
uniform uvec3 numClusters; // The fixed number of clusters in x y and z axes

layout(binding = 0) uniform sampler2D Depth;

//Output
layout(std430, binding = 1) buffer clusterActiveSSBO
{
    bool clusterActive[];
};

uint getClusterIndex(vec3 pixelCoord);
uint getDepthSlice(float z);
float linearDepth(float depthSample);

//We will evaluate the whole screen in one compute shader
//so each thread is equivalent to a pixel
void main()
{
    //Getting the depth value
    vec2 pixelID = vec2(gl_WorkGroupID.x, gl_WorkGroupID.y);
    vec2 screenCord = pixelID.xy / gl_NumWorkGroups.xy;
    float z = texture2D(Depth, screenCord).r;

    //Getting the linear cluster index value
    uint clusterID = getClusterIndex(vec3(pixelID.xy, z));
    clusterActive[clusterID] = true;
}

uint getClusterIndex(vec3 pixelCoord)
{
    uint clusterZVal = getDepthSlice(pixelCoord.z);

    uvec3 clusters = uvec3(uvec2(pixelCoord.xy / tileSizeInPx), clusterZVal);
    uint clusterIndex = clusters.x +
        numClusters.x * clusters.y +
        (numClusters.x * numClusters.y) * clusters.z;
    return clusterIndex;
}

uint getDepthSlice(float depth)
{
    float scale = 1.0;
    float bias = 0.0;
    return uint(max(log2(linearDepth(depth)) * scale + bias, 0.0));
}

float linearDepth(float depthSample)
{
    float depthRange = 2.0 * depthSample - 1.0;
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    return linear;
}
