vec3 CalculateSunLight(LightData light, vec3 normal, vec3 color, vec3 viewDir, float shininess)
{
	float intensity = light.Color.a;

	vec3 lightToPixelDir = normalize(light.Direction.xyz);
    /* Diffuse shading */
    float diff = max(dot(normal, lightToPixelDir), 0.0);
    /* Specular shading */
    vec3 reflectDir = reflect(-lightToPixelDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    /* Combine results */
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
    /* Diffuse shading */
    float diff = max(dot(normal, lightToPixelDir), 0.0);
    /* Specular shading */
    vec3 reflectDir = reflect(-lightToPixelDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    /* Attenuation */
    float distance = length(lightToPixel);
	float attenuation = DistanceAttenuation(distance, innerRadius, outerRadius, exponent, intensity);

    /* Combine results */
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
    /* Diffuse shading */
    float diff = max(dot(normal, lightToPixelDir), 0.0);
    /* Specular shading */
    vec3 reflectDir = reflect(-lightToPixelDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    /* Attenuation */
    float distance = length(lightToPixel);
    float attenuation = DistanceAttenuation(distance, 0.0, range, exponent, intensity);
    attenuation *= SpotAttenuation(lightToPixelDir, light.Direction.xyz, innerRadius, outerRadius);

    /* Combine results */
    vec3 ambient = 0.05*color;
    vec3 diffuse = light.Color.xyz*diff*color;
    vec3 specular = vec3(0.3)*light.Color.xyz*spec;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}