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

layout(set = 2, std430, binding = 3) readonly buffer LightSSBO
{
    LightData Lights[];
};

layout(set = 2, std430, binding = 4) readonly buffer LightSpaceTransformsSSBO
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

const uint Sun = 1;
const uint Point = 2;
const uint Spot = 3;

float DistanceAttenuation(float distance, float minRange, float maxRange, float exponent, float intensity)
{
	if (distance > maxRange) return 0.0;
	float distanceBeyondMinimum = max(distance - minRange, 0.0);
	if (distanceBeyondMinimum <= 0.0001) return 1.0;
	float range = maxRange - minRange;

	float distanceAttenuation = pow(clamp(1 - pow((distanceBeyondMinimum/range), 4.0), 0.0, 1.0), 2.0);
	float squareAttenuation = (1.0  + pow(distanceBeyondMinimum, exponent));
	return clamp(distanceAttenuation/squareAttenuation*intensity, 0.0, 1.0);
}

float SpotAttenuation(vec3 lightToPixelDir, vec3 lightDir, float innerCos, float outerCos)
{
    float angleCos = dot(lightToPixelDir, lightDir);
    return angleCos > outerCos ? smoothstep(outerCos, innerCos, angleCos) : 0.0;
}