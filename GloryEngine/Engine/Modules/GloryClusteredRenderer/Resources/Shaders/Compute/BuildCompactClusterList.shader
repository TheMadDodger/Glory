#version 430 core
layout(local_size_x = 1, local_size_y = 1) in;

//Input
layout(std430, binding = 1) buffer clusterActiveSSBO
{
    bool clusterActive[];
};

//Output
layout(std430, binding = 2) buffer uniqueActiveClustersSSBO
{
    uint globalActiveClusterCount;
    uint uniqueActiveClusters[];
};

//One compute shader for all clusters, one cluster per thread
void main()
{
    uint clusterIndex = uint(gl_GlobalInvocationID);
    if (clusterActive[clusterIndex])
    {
        uint offset = atomicAdd(globalActiveClusterCount, 1);
        uniqueActiveClusters[offset] = clusterIndex;
    }
}
