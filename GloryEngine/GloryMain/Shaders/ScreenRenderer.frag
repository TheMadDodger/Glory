#version 400

in vec2 Coord;
layout(location = 0) out vec4 out_Color;
layout (binding = 0) uniform sampler2D Color;
layout (binding = 1) uniform sampler2D Position;
layout (binding = 2) uniform sampler2D Normal;
layout (binding = 3) uniform sampler2D Depth;

uniform vec3 eyeDirection;

vec3 light = vec3(-0.707, 0.0, 0.707);
vec3 specularColor = vec3(1.0, 1.0, 1.0);

void main()
{
	//out_Color = vec4(0.0, 1.0, 0.0, 1.0);

	vec3 lightVec = normalize(light);
	vec3 normal = texture2D(Normal, Coord).xyz;
	float diffuse = dot(-lightVec, normal);

	vec3 lightReflected = reflect(lightVec, normal);
	float specular = pow(dot(-lightReflected, eyeDirection), 5);

	vec4 color = texture2D(Color, Coord);
	out_Color = vec4(color.xyz * diffuse, 1.0);
}
