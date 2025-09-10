struct LightData
{
	vec3 Position;
	uint Type;
	vec4 Direction;
	vec4 Color;
	vec4 Data;
	uint ShadowsEnabled;
	float ShadowBias;
	float Padding1;
	float Padding2;
	vec4 ShadowCoords;
};

struct CompactLightData
{
	vec3 Position;
	uint Type;
	vec4 Direction;
	vec4 Data;
};

struct LightGridElement
{
    uint Offset;
    uint Count;
};

layout(set = 2, std430, binding = 3) buffer LightSSBO
{
    LightData Lights[];
};

layout(set = 2, std430, binding = 4) buffer LightSpaceTransformsSSBO
{
    mat4 LightSpaceTransforms[];
};

layout(set = 3, std430, binding = 5) buffer LightIndexSSBO
{
    uint GlobalIndexCount;
    uint GlobalLightIndexList[];
};

layout(set = 3, std430, binding = 6) buffer LightGridSSBO
{
    LightGridElement LightGrid[];
};

layout(set = 3, std430, binding = 7) buffer LightDistanceSSBO
{
    uint LightDepthSlices[];
};