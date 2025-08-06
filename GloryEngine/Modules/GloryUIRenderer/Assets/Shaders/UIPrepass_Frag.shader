#type frag
#version 450

layout(binding = 0) uniform sampler2D texSampler;
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec4 outColor;

layout(binding = 1, std430) readonly buffer PropertiesSSBO
{
	vec4 Color;
    vec4 HasTexture;
} Properties;

void main()
{    
    vec4 sampled = Properties.HasTexture.x == 1.0 ? texture(texSampler, inTexCoord) : vec4(1.0, 1.0, 1.0, 1.0);
    outColor = vec4(inColor, 1.0)*sampled*Properties.Color;
}