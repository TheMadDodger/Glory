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

layout(set = 2, std430, binding = 3) buffer lightSSBO
{
    LightData Lights[];
};

layout(set = 3, std430, binding = 4) buffer lightIndexSSBO
{
    uint GlobalIndexCount;
    uint GlobalLightIndexList[];
};

layout(set = 3, std430, binding = 5) buffer lightGridSSBO
{
    LightGridElement LightGrid[];
};

layout(set = 3, std430, binding = 6) buffer lightDistanceSSBO
{
    uint LightDepthSlices[];
};
