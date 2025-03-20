#type vert
#version 450

layout (location = 0) in vec3 aPos;
layout(location = 0) out vec3 TexCoords;

#include "internal/ObjectData.glsl"

void main()
{
    TexCoords = aPos;
    gl_Position = Object.proj * Object.view * vec4(aPos, 1.0);
}
