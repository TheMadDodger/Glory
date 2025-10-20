#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

#define FEATURE_TEXTURED
#define FEATURE_RECEIVE_SHADOWS

#include "Internal/RenderConstants.glsl"

struct Material
{
	vec4 Color;
	float Shininess;
};

#include "Internal/Camera.glsl"
#include "Internal/Light.glsl"
#include "Internal/Material.glsl"
#include "Internal/Phong.glsl"
#include "Internal/DepthHelpers.glsl"

#ifdef WITH_RECEIVE_SHADOWS
layout (set = 5, binding = 0) uniform sampler2D ShadowAtlas;
//layout (set = 5, binding = 1) uniform samplerCube IrradianceMap;
#endif

#ifdef WITH_TEXTURED
#include "internal/Textured.glsl"

layout(set = 6, binding = 1) uniform sampler2D texSampler;
layout(set = 6, binding = 2) uniform sampler2D normalSampler;
layout(set = 6, binding = 3) uniform sampler2D shininessSampler;
#endif

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 inWorldPosition;
layout(location = 2) in vec3 inViewPosition;
layout(location = 3) in vec4 inColor;
#ifdef WITH_TEXTURED
layout(location = 4) in mat3 TBN;
#else
layout(location = 4) in vec3 inNormal;
#endif

layout(location = 0) out uvec4 outID;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec4 outNormal;

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

void main()
{
	Material mat = GetMaterial();

#ifdef WITH_TEXTURED
	vec4 baseColor = TextureEnabled(0) ? texture(texSampler, fragTexCoord)*inColor.a : mat.Color*inColor.a;
	if (baseColor.a == 0.0) discard;

	vec3 normal = TextureEnabled(1) ? normalize(TBN*(texture(normalSampler, fragTexCoord).xyz*2.0 - 1.0)) : TBN[2];
	float shininess = TextureEnabled(2) ? texture(shininessSampler, fragTexCoord).r : mat.Shininess;
#else
	vec4 baseColor = mat.Color*inColor.a;
	vec3 normal = inNormal;
	float shininess = mat.Shininess;
#endif

	outID = Constants.ObjectID;
	outNormal = vec4((normalize(normal) + 1.0)*0.5, 1.0);

	CameraData camera = CurrentCamera();
    vec3 cameraPos = camera.ViewInverse[3].xyz;
	vec3 color = baseColor.xyz;
    vec3 viewDir = normalize(cameraPos - inWorldPosition);

	uint zTile = uint((log(abs(inViewPosition.z) / camera.zNear) * Constants.GridSize.z) / log(camera.zFar / camera.zNear));
    vec2 tileSize = camera.Resolution / Constants.GridSize.xy;
    uvec3 tile = uvec3(gl_FragCoord.xy / tileSize, zTile);
    uint clusterID = tile.x + (tile.y * Constants.GridSize.x) + (tile.z * Constants.GridSize.x * Constants.GridSize.y);

	uint offset = LightGrid[clusterID].Offset;
	uint count = LightGrid[clusterID].Count;

	vec3 shadedColor = 0.05*color;
	for (uint i = 0; i < count; ++i)
	{
		uint indexListIndex = offset + i;
		uint lightIndex = GlobalLightIndexList[indexListIndex];
		LightData light = Lights[lightIndex];

#ifdef WITH_RECEIVE_SHADOWS
		vec4 fragPosLightSpace = LightSpaceTransforms[lightIndex]*vec4(inWorldPosition, 1.0);
		vec3 lightDir = normalize(inWorldPosition - inWorldPosition);
		float shadow = light.ShadowsEnabled == 1 ? 1.0 - ShadowCalculation(fragPosLightSpace, light, normal, lightDir) : 1.0;
		if (shadow == 0.0)
			continue;
#else
		float shadow = 1.0;
#endif
		if (light.Type == Sun)
			shadedColor += CalculateSunLight(light, normal, color, viewDir, shininess)*shadow;
		else if (light.Type == Point)
			shadedColor += CalcPointLight(light, normal, inWorldPosition, color, viewDir, shininess)*shadow;
		else if (light.Type == Spot)
			shadedColor += CalcSpotLight(light, normal, inWorldPosition, color, viewDir, shininess)*shadow;
	}
	outColor = vec4(shadedColor, baseColor.a);
}
