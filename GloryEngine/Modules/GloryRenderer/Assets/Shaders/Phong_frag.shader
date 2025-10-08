#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Internal/RenderConstants.glsl"

struct Material
{
	vec4 Color;
	float Shininess;
};

#include "Internal/Camera.glsl"
#include "Internal/Light.glsl"
#include "Internal/Material.glsl"
//#include "Internal/DepthHelpers.glsl"

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 inWorldPosition;
layout(location = 2) in vec4 inColor;

layout(location = 0) out uvec4 outID;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec4 outNormal;

const float PI = 3.14159265359;

const float Constant = 1.0;
const float Linear = 0.0014;
const float Quadratic = 0.000007;

vec3 CalculateSunLight(LightData light, vec3 normal, vec3 color, vec3 viewDir, float shininess)
{
	float intensity = light.Color.a;

	vec3 lightToPixelDir = normalize(light.Direction.xyz);
    /* diffuse shading */
    float diff = max(dot(normal, lightToPixelDir), 0.0);
    /* specular shading */
    vec3 reflectDir = reflect(-lightToPixelDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    /* combine results */
    vec3 ambient = 0.05*color;
    vec3 diffuse = light.Color.xyz*diff*color;
    vec3 specular = vec3(0.3)*light.Color.xyz*spec;
    return (ambient + diffuse + specular)*intensity;
}

vec3 CalcPointLight(LightData light, vec3 normal, vec3 worldPosition, vec3 color, vec3 viewDir, float shininess)
{
	vec3 position = light.Position.xyz;
    float innerRadius = light.Data.x;
	float outerRadius = light.Data.y;
	float exponent = light.Data.w;
	float intensity = light.Color.a;

    vec3 lightToPixel = position - worldPosition;
    vec3 lightToPixelDir = normalize(lightToPixel);
    /* diffuse shading */
    float diff = max(dot(normal, lightToPixelDir), 0.0);
    /* specular shading */
    vec3 reflectDir = reflect(-lightToPixelDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    /* attenuation */
    float distance = length(lightToPixel);
	float attenuation = DistanceAttenuation(distance, innerRadius, outerRadius, exponent, intensity);

    /* combine results */
    vec3 ambient = 0.05*color;
    vec3 diffuse = light.Color.xyz*diff*color;
    vec3 specular = vec3(0.3)*light.Color.xyz*spec;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(LightData light, vec3 normal, vec3 worldPosition, vec3 color, vec3 viewDir, float shininess)
{
	vec3 position = light.Position.xyz;
	float innerRadius = light.Data.x;
	float outerRadius = light.Data.y;
	float range = light.Data.z;
	float exponent = light.Data.w;
	float intensity = light.Color.a;

	vec3 lightToPixel = position - worldPosition;
    vec3 lightToPixelDir = normalize(lightToPixel);
    /* diffuse shading */
    float diff = max(dot(normal, lightToPixelDir), 0.0);
    /* specular shading */
    vec3 reflectDir = reflect(-lightToPixelDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    /* attenuation */
    float distance = length(lightToPixel);
    float attenuation = DistanceAttenuation(distance, 0.0, range, exponent, intensity);
    attenuation *= SpotAttenuation(lightToPixelDir, light.Direction.xyz, innerRadius, outerRadius);

    /* combine results */
    vec3 ambient = 0.05*color;
    vec3 diffuse = light.Color.xyz*diff*color;
    vec3 specular = vec3(0.3)*light.Color.xyz*spec;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

void main()
{
	Material mat = GetMaterial();

	outID = Constants.ObjectID;
	outNormal = vec4((normalize(normal) + 1.0)*0.5, 1.0);

	CameraData camera = CurrentCamera();
    vec3 cameraPos = camera.ViewInverse[3].xyz;
	vec3 color = (inColor*mat.Color).xyz;
    vec3 viewDir = normalize(cameraPos - inWorldPosition);

	vec3 shadedColor = 0.05*color;
	for (uint i = 0; i < Constants.LightCount; ++i)
	{
		LightData light = Lights[i];

		if (light.Type == Sun)
			shadedColor += CalculateSunLight(light, normal, color, viewDir, mat.Shininess);
		else if (light.Type == Point)
			shadedColor += CalcPointLight(light, normal, inWorldPosition, color, viewDir, mat.Shininess);
		else if (light.Type == Spot)
			shadedColor += CalcSpotLight(light, normal, inWorldPosition, color, viewDir, mat.Shininess);
	}
	outColor = vec4(shadedColor, inColor.a*mat.Color.a);
}
