#type frag
#version 450

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec4 outColor;

//layout(binding = 1, std430) buffer PropertiesSSBO
//{
//	vec4 Color;
//    vec4 HasTexture;
//} Properties;

void main()
{
    outColor = vec4(0.1, 0.1, 0.1, 0.7);
}