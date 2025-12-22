#type frag
#version 450

#include "Internal/UIConstants.glsl"

layout(set = 1, binding = 0) uniform sampler2D Color;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 outColor;

void main()
{    
    vec4 sampled = Constants.HasTexture == 1 ? texture(Color, inTexCoord) : vec4(1.0, 1.0, 1.0, 1.0);
    outColor = inColor*sampled;
}