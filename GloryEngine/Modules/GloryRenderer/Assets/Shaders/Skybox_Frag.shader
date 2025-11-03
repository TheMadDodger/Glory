#type frag
#version 450

layout(location = 0) in vec3 TexCoords;
layout(location = 0) out uvec4 outID;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec4 outNormal;

layout(set = 1, binding = 0) uniform samplerCube Color;

void main()
{
    outID = uvec4(0);
    outColor = texture(Color, TexCoords);
    outNormal = vec4(0.0);
}
