#type frag
#version 450

layout(location = 0) in vec3 TexCoords;
layout(location = 2) out vec4 outColor;
layout(location = 5) out vec4 outData;

layout(binding = 0) uniform samplerCube skybox;

void main()
{
    outColor = texture(skybox, TexCoords);
    outData.a = 0.0;
}
