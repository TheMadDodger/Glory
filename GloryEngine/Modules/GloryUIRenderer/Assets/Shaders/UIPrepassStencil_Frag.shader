#type frag
#version 450

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 outColor;

void main()
{    
    outColor = vec4(0.0, 0.0, 0.0, 0.0);
}