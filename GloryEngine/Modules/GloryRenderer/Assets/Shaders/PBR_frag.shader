#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

#define FEATURE_TEXTURED
#define FEATURE_RECEIVE_SHADOWS
#define FEATURE_TRANSPARENCY

struct Material
{
	vec4 Color;
	float AmbientOcclusion;
	float RoughnessFactor;
	float MetallicFactor;
};

#ifdef WITH_RECEIVE_SHADOWS
layout (set = 5, binding = 0) uniform sampler2D ShadowAtlas;
//layout (set = 5, binding = 1) uniform samplerCube IrradianceMap;
#endif

#include "Internal/RenderConstants.glsl"
#include "Internal/Camera.glsl"
#include "Internal/Light.glsl"
#include "Internal/Material.glsl"
#include "Internal/PBR.glsl"
#include "Internal/DepthHelpers.glsl"

#ifdef WITH_TEXTURED
#include "internal/Textured.glsl"
layout(set = 6, binding = 1) uniform sampler2D texSampler;
layout(set = 6, binding = 2) uniform sampler2D normalSampler;
layout(set = 6, binding = 3) uniform sampler2D ambientSampler;
layout(set = 6, binding = 4) uniform sampler2D roughnessSampler;
layout(set = 6, binding = 5) uniform sampler2D metalnessSampler;
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

void main()
{
	Material mat = GetMaterial();

#ifdef WITH_TEXTURED
	vec4 baseColor = TextureEnabled(0) ? texture(texSampler, fragTexCoord) : vec4(1.0);

	baseColor = (TextureEnabled(0) ? vec4(pow(baseColor.rgb, vec3(2.2)), baseColor.a) : mat.Color)*inColor;
	vec3 normal = TextureEnabled(1) ? normalize(TBN*(texture(normalSampler, fragTexCoord).xyz*2.0 - 1.0)) : TBN[2];
	float ao = TextureEnabled(2) ? texture(ambientSampler, fragTexCoord).r : mat.AmbientOcclusion;
	float roughness = TextureEnabled(3) ? texture(roughnessSampler, fragTexCoord).g : mat.RoughnessFactor;
	float metallic = TextureEnabled(4) ? texture(metalnessSampler, fragTexCoord).b : mat.MetallicFactor;
#else
	vec4 baseColor = inColor*mat.Color;
	vec3 normal = inNormal;
	float ao = mat.AmbientOcclusion;
	float roughness = mat.RoughnessFactor;
	float metallic = mat.MetallicFactor;
#endif

#ifdef WITH_TRANSPARENCY
	if (baseColor.a < 0.1) discard;
#endif

	outID = Constants.ObjectID;
	outNormal = vec4((normalize(normal) + 1.0)*0.5, 1.0);

	CameraData camera = CurrentCamera();
    vec3 cameraPos = camera.ViewInverse[3].xyz;
	vec3 color = baseColor.xyz;
    vec3 viewDir = normalize(cameraPos - inWorldPosition);

	uint zTile = uint((log(abs(inViewPosition.z)/camera.zNear)*Constants.GridSize.z)/log(camera.zFar/camera.zNear));
    vec2 tileSize = camera.Resolution/Constants.GridSize.xy;
    uvec3 tile = uvec3(gl_FragCoord.xy/tileSize, zTile);
    uint clusterID = tile.x + (tile.y*Constants.GridSize.x) + (tile.z*Constants.GridSize.x*Constants.GridSize.y);

	uint offset = LightGrid[clusterID].Offset;
	uint count = LightGrid[clusterID].Count;

	vec3 Lo = vec3(0.0);
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
			Lo += CalculateSunLight(light, normal, color, viewDir, roughness, metallic);
		else if (light.Type == Point)
			Lo += CalcPointLight(light, normal, inWorldPosition, color, viewDir, roughness, metallic);
		else if (light.Type == Spot)
			Lo += CalcSpotLight(light, normal, inWorldPosition, color, viewDir, roughness, metallic);
	}

	/* Ambient lighting */
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, color, metallic);
    vec3 kS = FresnelSchlick(max(dot(normal, viewDir), 0.0), F0, roughness);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    //vec3 irradiance = TextureEnabled(6) ? texture(IrradianceMap, normal).rgb : vec3(0.03);
    vec3 irradiance = vec3(0.03);
    vec3 diffuse = irradiance*color;
    vec3 ambient = (kD*diffuse)*ao;
	vec3 fragColor = ambient + Lo;

	/* Gamma correction */
	fragColor = fragColor/(fragColor + vec3(1.0));
	fragColor = pow(fragColor, vec3(1.0/2.2));
	outColor = vec4(fragColor, baseColor.a);
}
