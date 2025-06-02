#type vert

#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "internal/ObjectData.glsl"

#define FEATURE_TRANSPARENT_TEXTURED

layout(location = 0) in vec3 inPosition;

#ifdef WITH_TRANSPARENT_TEXTURED
layout(location = 4) in vec2 inTexCoord;
layout(location = 0) out vec2 fragTexCoord;
#endif

void main()
{
	gl_Position = Object.view * Object.model * vec4(inPosition, 1.0);
	fragTexCoord = inTexCoord;
}
