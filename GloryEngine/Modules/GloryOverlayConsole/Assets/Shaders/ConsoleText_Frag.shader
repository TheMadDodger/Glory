#type frag
#version 450

layout(set = 0, binding = 0) uniform sampler2D Color;
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec4 outColor;

void main()
{
    vec4 sampled = texture(Color, inTexCoord);
    outColor = vec4(inColor, sampled.r);
}