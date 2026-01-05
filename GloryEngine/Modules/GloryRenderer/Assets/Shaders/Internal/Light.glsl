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

#ifdef WITH_RECEIVE_SHADOWS
float ShadowCalculation(vec4 fragPosLightSpace, LightData lightData, vec3 normal, vec3 lightDir)
{
    /* perform perspective divide */
    vec3 projCoords = fragPosLightSpace.xyz/fragPosLightSpace.w;
    /* transform to [0,1] range */
    projCoords = projCoords*0.5 + 0.5;
    /* get depth of current fragment from light's perspective */
    float currentDepth = projCoords.z;

	/* calculate texture coords */
	vec4 shadowCoords = lightData.ShadowCoords;
	vec2 coordRanges = vec2(shadowCoords.z - shadowCoords.x, shadowCoords.w - shadowCoords.y);
	vec2 actualCoords = vec2(shadowCoords.x + coordRanges.x*projCoords.x, shadowCoords.y + coordRanges.y*projCoords.y);

	/* Calculate bias based on surface normal */
	float bias = lightData.ShadowBias;
	float surfaceBias = max(bias*(1.0 - dot(normal, lightDir)), bias*0.01);

	/* PCF samples */
	float shadow = 0.0;
	vec2 texelSize = 1.0/textureSize(ShadowAtlas, 0);
	int sampleCounts = 1;
	float totalSamples = pow((float(sampleCounts)*2.0 + 1.0), 2.0);
	for (int x = -sampleCounts; x <= sampleCounts; ++x)
	{
		for (int y = -sampleCounts; y <= sampleCounts; ++y)
		{
			vec2 pcfCoord = actualCoords + vec2(x, y)*texelSize;
			pcfCoord = clamp(pcfCoord, shadowCoords.xy, shadowCoords.zw);
			float pcfDepth = texture(ShadowAtlas, pcfCoord).r; 
			shadow += currentDepth - surfaceBias > pcfDepth ? 1.0 : 0.0;
		}    
	}
	shadow /= totalSamples;
    return shadow;
}
#endif