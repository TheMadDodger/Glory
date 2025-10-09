#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

#define FEATURE_TEXTURED

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
//#include "Internal/DepthHelpers.glsl"

#ifdef WITH_TEXTURED
#include "internal/Textured.glsl"

layout(set = 6, binding = 0) uniform sampler2D texSampler;
layout(set = 6, binding = 1) uniform sampler2D normalSampler;
layout(set = 6, binding = 2) uniform sampler2D shininessSampler;
#endif

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 inWorldPosition;
layout(location = 2) in vec4 inColor;
#ifdef WITH_TEXTURED
layout(location = 3) in mat3 TBN;
#else
layout(location = 3) in vec3 inNormal;
#endif

layout(location = 0) out uvec4 outID;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec4 outNormal;

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

	vec3 shadedColor = 0.05*color;
	for (uint i = 0; i < Constants.LightCount; ++i)
	{
		LightData light = Lights[i];
		if (light.Type == Sun)
			shadedColor += CalculateSunLight(light, normal, color, viewDir, shininess);
		else if (light.Type == Point)
			shadedColor += CalcPointLight(light, normal, inWorldPosition, color, viewDir, shininess);
		else if (light.Type == Spot)
			shadedColor += CalcSpotLight(light, normal, inWorldPosition, color, viewDir, shininess);
	}
	outColor = vec4(shadedColor, baseColor.a);
}
