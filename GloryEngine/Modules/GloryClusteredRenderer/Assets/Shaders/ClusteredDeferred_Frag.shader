#type frag
#version 450
#extension GL_ARB_separate_shader_objects : enable

#define FEATURE_PBR

layout(location = 0) in vec2 Coord;
layout(location = 0) out vec4 out_Color;
layout (binding = 0) uniform sampler2D Debug;
layout (binding = 1) uniform sampler2D Color;
layout (binding = 2) uniform sampler2D Normal;
layout (binding = 3) uniform sampler2D AO;
layout (binding = 4) uniform sampler2D Data;
layout (binding = 5) uniform sampler2D Depth;

#include "Internal/DepthHelpers.glsl"

const vec3 depthSliceColors[8] = vec3[8](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(1.0, 0.0, 1.0),
	vec3(1.0, 1.0, 0.0),
	vec3(0.0, 1.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(0.5, 0.5, 0.5)
);

const float PI = 3.14159265359;

struct LightData
{
	/* Type is in the w value */
    vec4 Position;
	vec4 Direction;
	vec4 Color;
	vec4 Data;
};

struct LightGridElement
{
    uint Offset;
    uint Count;
};

struct VolumeTileAABB
{
    vec4 MinPoint;
    vec4 MaxPoint;
};

layout(std430, binding = 1) buffer clusterAABB
{
    VolumeTileAABB Cluster[];
};

layout(std430, binding = 3) buffer lightSSBO
{
    LightData Lights[];
};

layout(std430, binding = 4) buffer lightIndexSSBO
{
	uint GlobalIndexCount;
    uint GlobalLightIndexList[];
};

layout(std430, binding = 5) buffer lightGridSSBO
{
    LightGridElement LightGrid[];
};

layout(std430, binding = 6) buffer ssaoSettings
{
    int AOEnabled;
    int Dirty;
    float SampleRadius;
    float SampleBias;
    int KernelSize;
    int BlurType;
    int BlurSize;
    float Separation;
    int BinsSize;
    float Magnitude;
    float Contrast;
};

const vec3 GoodColor = vec3(0.0, 1.0, 0.0);
const vec3 BadColor = vec3(1.0, 0.0, 0.0);

const uint Sun = 1;
const uint Point = 2;
const uint Spot = 3;

#ifdef WITH_PBR

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 CalculateLighting(LightData light, vec3 normal, vec3 color, vec3 worldPosition, vec3 CameraPos, vec3 V, float roughness, float metallic)
{
	vec3 lightPos = light.Position.xyz;
	float lightType = light.Position.w;
	vec3 direction = light.Direction.xyz;
	float innerRadius = light.Data.x;
	float outerRadius = light.Data.y;
	float range = light.Data.z;
	float intensity = light.Data.w;
	vec3 lightColor = light.Color.xyz;

	vec3 L;
	float attenuation = 0.0;

	if (lightType == Sun)
	{
		L = direction;
		attenuation = 1.0;
	}
	if (lightType == Point)
	{
		L = normalize(lightPos - worldPosition);
		float distance = length(lightPos - worldPosition);
		float falloffRadius = max(outerRadius - innerRadius, 0.001);
		float distanceFalloff = clamp(distance - innerRadius, 0.0, distance);
		attenuation = clamp(1.0 - (distanceFalloff / falloffRadius), 0.0, 1.0);
	}
	if (lightType == Spot)
	{
		vec3 lightVec = lightPos - worldPosition;
		float projected = dot(lightVec, direction);
		if (projected <= 0.0) return vec3(0.0);
		if (projected > range) projected = range;
		vec3 pointInCone = lightPos - projected*direction;
		vec3 centerToPos = worldPosition - pointInCone;
		float distance = length(centerToPos);

		float innerRadians = innerRadius*PI/180.0;
		float outerRadians = outerRadius*PI/180.0;
		float inner = projected*tan(innerRadians/2.0);
		float outer = projected*tan(outerRadians/2.0);
		float falloffRadius = max(outer - inner, 0.001);
		float distanceFalloff1 = clamp(distance - inner, 0.0, distance);

		attenuation = clamp(1.0 - (distanceFalloff1 / falloffRadius), 0.0, 1.0);
		L = normalize(lightVec);
	}

	vec3 radiance = lightColor * attenuation * intensity;

	vec3 H = normalize(V + L);
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, color, metallic);
	vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

	float NDF = DistributionGGX(normal, H, roughness);
	float G = GeometrySmith(normal, V, L, roughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0)  + 0.0001;
	vec3 specular = numerator / denominator;

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	kD *= 1.0 - metallic;

	float NdotL = max(dot(normal, L), 0.0);
	return (kD * color / PI + specular) * radiance * NdotL;
}

void main()
{
	vec3 debug = texture(Debug, Coord).xyz;
	if (debug.r > 0.0 || debug.g > 0.0 || debug.b > 0.0)
	{
		out_Color = vec4(debug, 1.0);
		return;
	}

	vec3 CameraPos = ViewInverse[3].xyz;

	vec3 color = texture(Color, Coord).xyz;
	vec4 metallicRoughnessAO = texture(Data, Coord);
	float ssao = AOEnabled == 1 ? Magnitude*pow(texture(AO, Coord).x, Contrast) : 1.0;
	ssao = min(ssao, 1.0);
	float ao = AOEnabled == 1 ? ssao : metallicRoughnessAO.r;
	float roughness = metallicRoughnessAO.g;
	float metallic = metallicRoughnessAO.b;
	float shadingEnabled = metallicRoughnessAO.a;
	if (shadingEnabled == 0.0)
	{
		out_Color = vec4(color, 1.0);
		return;
	}

	vec3 normal = normalize(texture(Normal, Coord).xyz*2.0 - 1.0);

	float depth = texture(Depth, Coord).r;
	vec3 worldPosition = WorldPosFromDepth(depth);

	vec2 pixelID = Coord * ScreenDimensions;
	uint clusterID = GetClusterIndex(vec3(pixelID.xy, depth));

	/* View vector */
	vec3 V = normalize(CameraPos - worldPosition);

	vec3 Lo = vec3(0.0);

	/* Find light array */
	uint offset = LightGrid[clusterID].Offset;
	uint count = LightGrid[clusterID].Count;
	for (uint i = 0; i < count; i++)
	{
		uint indexListIndex = offset + i;
		uint lightIndex = GlobalLightIndexList[indexListIndex];
		Lo += CalculateLighting(Lights[lightIndex], normal, color, worldPosition, CameraPos, V, roughness, metallic);
	}

	vec3 ambient = vec3(0.03) * color * ao;
	vec3 fragColor   = ambient + Lo;

	/* Gamma correction */
	fragColor = fragColor / (fragColor + vec3(1.0));
	fragColor = pow(fragColor, vec3(1.0/2.2));

	out_Color = vec4(fragColor, 1.0);
}

#else

vec3 CalculateLighting(LightData light, vec3 normal, vec3 color, vec3 worldPosition, vec3 viewDir, float specularIntensity)
{
	vec3 lightPos = light.Position.xyz;
	float lightType = light.Position.w;
	vec3 direction = normalize(light.Direction.xyz);
	float innerRadius = light.Data.x;
	float outerRadius = light.Data.y;
	float range = light.Data.z;
	float intensity = light.Data.w;
	vec3 lightColor = light.Color.xyz;
	float lightColorAlpha = light.Color.a;

	vec3 lightDir;
	float attenuation = 0.0;

	if (lightType == Sun)
	{
		lightDir = direction;
		attenuation = 1.0;
	}
	if (lightType == Point)
	{
		vec3 lightVec = lightPos - worldPosition;
		float distance = length(lightVec);
		float falloffRadius = max(outerRadius - innerRadius, 0.001);
		float distanceFalloff = clamp(distance - innerRadius, 0.0, distance);
		attenuation = clamp(1.0 - (distanceFalloff / falloffRadius), 0.0, 1.0); //pow(clamp(1 - pow((distance / radius), 4.0), 0.0, 1.0), 2.0)/(1.0  + (distance * distance));
		lightDir = normalize(lightVec);
	}
	if (lightType == Spot)
	{
		vec3 lightVec = lightPos - worldPosition;
		float projected = dot(lightVec, direction);
		if (projected <= 0.0) return vec3(0.0);
		if (projected > range) projected = range;
		vec3 pointInCone = lightPos - projected*direction;
		vec3 centerToPos = worldPosition - pointInCone;
		float distance = length(centerToPos);

		float innerRadians = innerRadius*PI/180.0;
		float outerRadians = outerRadius*PI/180.0;
		float inner = projected*tan(innerRadians/2.0);
		float outer = projected*tan(outerRadians/2.0);
		float falloffRadius = max(outer - inner, 0.001);
		float distanceFalloff1 = clamp(distance - inner, 0.0, distance);

		attenuation = clamp(1.0 - (distanceFalloff1 / falloffRadius), 0.0, 1.0);
		lightDir = normalize(lightVec);
	}

	vec3 diffuse = max(dot(normal, lightDir), 0.0) * color * intensity * lightColor * lightColorAlpha;

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularIntensity * spec * lightColor;

	return diffuse * attenuation + specular * attenuation;
}

void main()
{
    vec3 CameraPos = ViewInverse[3].xyz;

	vec3 debug = texture(Debug, Coord).xyz;
	if (debug.r > 0.0 || debug.g > 0.0 || debug.b > 0.0)
	{
		out_Color = vec4(debug, 1.0);
		return;
	}

	vec3 color = texture(Color, Coord).xyz;
	vec4 metallicRoughnessAO = texture(Data, Coord);
	float specularIntensity = metallicRoughnessAO.g;
	float shadingEnabled = metallicRoughnessAO.a;
	if (shadingEnabled == 0.0)
	{
		out_Color = vec4(color, 1.0);
		return;
	}
	vec3 normal = texture(Normal, Coord).xyz*2.0 - 1.0;
	float ssao = AOEnabled == 1 ? Magnitude*pow(texture(AO, Coord).x, Contrast) : 1.0;
	ssao = min(ssao, 1.0);
	float depth = texture(Depth, Coord).r;
	vec3 worldPosition = WorldPosFromDepth(depth);

	vec2 pixelID = Coord * ScreenDimensions;
	uint clusterID = GetClusterIndex(vec3(pixelID.xy, depth));

	uint clusterColorIndex = uint(mod(clusterID, 5));
	uint totalClusters = TileSizes.x * TileSizes.y * TileSizes.z;
	float clusterFrac = float(clusterColorIndex) / float(totalClusters);

	uint clusterZVal = GetDepthSlice(depth);
	vec3 clusterColor = depthSliceColors[uint(mod(clusterZVal, 5))];

	uint offset = LightGrid[clusterID].Offset;
	uint count = LightGrid[clusterID].Count;
	float l = float(count) / 100.0;
	vec3 indicatorColor = mix(GoodColor, BadColor, l);

    vec3 viewDir = normalize(CameraPos - worldPosition);

	vec3 diffuseColor = color * 0.1;
	for (uint i = 0; i < count; i++)
	{
		uint indexListIndex = offset + i;
		uint lightIndex = GlobalLightIndexList[indexListIndex];

		diffuseColor += CalculateLighting(Lights[lightIndex], normal, color, worldPosition, viewDir, specularIntensity);
	}

	out_Color = vec4(diffuseColor*ssao, 1.0);
}

#endif
