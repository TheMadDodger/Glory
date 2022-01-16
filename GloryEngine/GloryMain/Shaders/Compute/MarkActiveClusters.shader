#version 430 core
layout(local_size_x = 1, local_size_y = 1) in;

//Shared between all clusters
uniform float zNear;
uniform float zFar;
uniform uint tileSizeInPx; // How many pixels a rectangular cluster takes in x and y
uniform uvec3 numClusters; // The fixed number of clusters in x y and z axes

layout(binding = 0) uniform sampler2D depth;

//Output
layout(std430, binding = 1) buffer clusterActiveSSBO
{
    bool clusterActive[];
};

uint getClusterIndex(vec3 pixelCoord);
uint getDepthSlice(float z);

//We will evaluate the whole screen in one compute shader
//so each thread is equivalent to a pixel
void main()
{
    //Getting the depth value
    vec2 pixelID = vec2(gl_WorkGroupID.x, gl_WorkGroupID.y);
    vec2 screenCord = pixelID.xy / gl_NumWorkGroups.xy;
    float z = texture2D(depth, screenCord).r;

    //Getting the linear cluster index value
    uint clusterID = getClusterIndex(vec3(pixelID.xy, z));
    clusterActive[clusterID] = true;
}

uint getClusterIndex(vec3 pixelCoord)
{
    // Uses equation (3) from Building a Cluster Grid section
    uint clusterZVal = getDepthSlice(pixelCoord.z);

    uvec3 clusters = uvec3(uvec2(pixelCoord.xy / tileSizeInPx), clusterZVal);
    uint clusterIndex = clusters.x +
        numClusters.x * clusters.y +
        (numClusters.x * numClusters.y) * clusters.z;
    return clusterIndex;
}

uint getDepthSlice(float z)
{
    float logZ = log(z);
    float logNear = log(zNear);
    float numSlices = numClusters.z;
    float logFarNear = log(zFar / zNear);
    float result = logZ* (numSlices / logFarNear) - ((numSlices * logNear) / logFarNear);
    return uint(result);
}