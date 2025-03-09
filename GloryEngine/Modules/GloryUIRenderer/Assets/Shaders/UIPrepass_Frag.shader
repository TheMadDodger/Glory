#type frag
#version 450

layout(binding = 0) uniform sampler2D texSampler;
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec4 outColor;

layout(binding = 1, std430) buffer PropertiesSSBO
{
	vec4 Color;
} Properties;

void main()
{    
    vec4 sampled = texture(texSampler, inTexCoord);
    outColor = vec4(inColor, 1.0)*sampled*Properties.Color;
}