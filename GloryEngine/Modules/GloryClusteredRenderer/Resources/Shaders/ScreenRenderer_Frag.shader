#version 450
#extension GL_ARB_separate_shader_objects : enable

in vec2 Coord;
layout(location = 0) out vec4 out_Color;
layout (binding = 0) uniform sampler2D Color;
layout (binding = 1) uniform sampler2D Normal;
layout (binding = 2) uniform sampler2D AO;
layout (binding = 3) uniform sampler2D Debug;
layout (binding = 4) uniform sampler2D Depth;

//uniform vec3 eyeDirection;

uniform float zNear;
uniform float zFar;

uint GetClusterIndex(vec3 pixelCoord);
uint GetDepthSlice(float z);
float LinearDepth(float depthSample);
vec3 WorldPosFromDepth(float depth);

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

struct PointLight
{
    vec4 Position;
    vec4 Color;
    uint Enabled;
    float Intensity;
    float Range;
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

layout(std430, binding = 2) buffer screenToView
{
    mat4 ProjectionInverse;
    mat4 ViewInverse;
    uvec4 TileSizes;
    uvec2 ScreenDimensions;
    float Scale;
    float Bias;
};

layout(std430, binding = 3) buffer lightSSBO
{
    PointLight Lights[];
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

void main()
{
	vec3 debug = texture2D(Debug, Coord).xzy;
	if (debug.r > 0.0 || debug.g > 0.0 || debug.b > 0.0)
	{
		out_Color = vec4(debug, 1.0);
		return;
	}

	vec3 color = texture2D(Color, Coord).xyz;
	vec3 normal = texture2D(Normal, Coord).xyz*2.0 - 1.0;
	float ssao = AOEnabled == 1 ? Magnitude*pow(texture2D(AO, Coord).x, Contrast) : 1.0;
	ssao = min(ssao, 1.0);
	float depth = texture2D(Depth, Coord).r;
	vec3 fragPosition = WorldPosFromDepth(depth);

	//vec3 lightReflected = reflect(lightVec, normal);
	//float specular = pow(dot(-lightReflected, eyeDirection), 5);

	vec2 pixelID = Coord * ScreenDimensions;
	uint clusterID = GetClusterIndex(vec3(pixelID.xy, depth));

	uint clusterColorIndex = uint(mod(clusterID, 5));
	uint totalClusters = TileSizes.x * TileSizes.y * TileSizes.z;
	float clusterFrac = float(clusterColorIndex) / float(totalClusters);

	uint clusterZVal = GetDepthSlice(depth);
	vec3 clusterColor = depthSliceColors[uint(mod(clusterZVal, 5.0))];

	uint offset = LightGrid[clusterID].Offset;
	uint count = LightGrid[clusterID].Count;
	float l = float(count) / 100.0;
	vec3 indicatorColor = mix(GoodColor, BadColor, l);

	vec3 diffuseColor = color * 0.1;
	for (uint i = 0; i < count; i++)
	{
		uint indexListIndex = offset + i;
		uint lightIndex = GlobalLightIndexList[indexListIndex];

		vec3 lightPos = Lights[lightIndex].Position.xyz;
		float intensity = Lights[lightIndex].Intensity;
		float radius = Lights[lightIndex].Range;
		vec3 lightColor = Lights[lightIndex].Color.xyz;
		float lightColorAlpha = Lights[lightIndex].Color.a;

		vec3 lightVec = lightPos - fragPosition;
		float distance = length(lightVec);
		float attenuation = clamp(1.0 - (distance / radius), 0.0, 1.0); //pow(clamp(1 - pow((distance / radius), 4.0), 0.0, 1.0), 2.0)/(1.0  + (distance * distance));
	
		vec3 lightDir = normalize(lightVec);
		vec3 diffuse = max(dot(normal, lightDir), 0.0) * color * intensity * lightColor * lightColorAlpha;
		diffuseColor += diffuse * attenuation;
	}

	out_Color = vec4(diffuseColor*ssao, 1.0);
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

uint GetDepthSlice(float depth)
{
	return uint(max(log2(LinearDepth(depth)) * Scale + Bias, 0.0));
}

float LinearDepth(float depthSample)
{
    float depthRange = 2.0 * depthSample - 1.0;
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    return linear;
}

vec3 WorldPosFromDepth(float depth)
{
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(Coord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = ProjectionInverse * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = ViewInverse * viewSpacePosition;

    return worldSpacePosition.xyz;
}
