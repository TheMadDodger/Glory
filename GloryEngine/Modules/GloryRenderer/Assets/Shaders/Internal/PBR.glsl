const float PI = 3.14159265359;

vec3 FresnelSchlick(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0)*pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num = a2;
    float denom = (NdotH2*(a2 - 1.0) + 1.0);
    denom = PI*denom*denom;
	
    return num/denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r)/8.0;

    float num = NdotV;
    float denom = NdotV*(1.0 - k) + k;

    return num/denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1*ggx2;
}

vec3 PBR_BRDF_CookTorrance(vec3 lightColor, vec3 viewDir, vec3 lightDir, vec3 normal, vec3 color, float roughness, float metallic, float attenuation, float intensity)
{
    vec3 radiance = lightColor*attenuation*intensity;

	vec3 H = normalize(viewDir + lightDir);
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, color, metallic);
	vec3 F = FresnelSchlick(max(dot(H, viewDir), 0.0), F0, roughness);

	float NDF = DistributionGGX(normal, H, roughness);
	float G = GeometrySmith(normal, viewDir, lightDir, roughness);

	vec3 numerator = NDF*G*F;
	float denominator = 4.0*max(dot(normal, viewDir), 0.0)*max(dot(normal, lightDir), 0.0) + 0.0001;
	vec3 specular = numerator/denominator;

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	kD *= 1.0 - metallic;

	float NdotL = max(dot(normal, lightDir), 0.0);
	return (kD*color/PI + specular)*radiance*NdotL;
}

vec3 CalculateSunLight(LightData light, vec3 normal, vec3 color, vec3 viewDir, float roughness, float metallic)
{
	float intensity = light.Color.a;
	vec3 lightToPixelDir = normalize(light.Direction.xyz);
    return PBR_BRDF_CookTorrance(light.Color.xyz, viewDir, lightToPixelDir, normal, color, roughness, metallic, 1.0, intensity);
}

vec3 CalcPointLight(LightData light, vec3 normal, vec3 worldPosition, vec3 color, vec3 viewDir, float roughness, float metallic)
{
	vec3 position = light.Position.xyz;
    float innerRadius = light.Data.x;
	float outerRadius = light.Data.y;
	float exponent = light.Data.w;
	float intensity = light.Color.a;

    vec3 lightToPixel = position - worldPosition;
    vec3 lightToPixelDir = normalize(lightToPixel);
    float distance = length(lightToPixel);
	float attenuation = DistanceAttenuation(distance, innerRadius, outerRadius, exponent, intensity);
	return PBR_BRDF_CookTorrance(light.Color.xyz, viewDir, lightToPixelDir, normal, color, roughness, metallic, attenuation, intensity);
}

vec3 CalcSpotLight(LightData light, vec3 normal, vec3 worldPosition, vec3 color, vec3 viewDir, float roughness, float metallic)
{
	vec3 position = light.Position.xyz;
	float innerRadius = light.Data.x;
	float outerRadius = light.Data.y;
	float range = light.Data.z;
	float exponent = light.Data.w;
	float intensity = light.Color.a;

	vec3 lightToPixel = position - worldPosition;
    vec3 lightToPixelDir = normalize(lightToPixel);
    float distance = length(lightToPixel);
    float attenuation = DistanceAttenuation(distance, 0.0, range, exponent, intensity);
    attenuation *= SpotAttenuation(lightToPixelDir, light.Direction.xyz, innerRadius, outerRadius);
	return PBR_BRDF_CookTorrance(light.Color.xyz, viewDir, lightToPixelDir, normal, color, roughness, metallic, attenuation, intensity);
}