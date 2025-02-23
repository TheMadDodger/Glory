#type frag
#version 450

layout(binding = 0) uniform sampler2D textSampler;
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec4 outColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(textSampler, inTexCoord).r);
    outColor = vec4(inColor, 1.0) * sampled;
}