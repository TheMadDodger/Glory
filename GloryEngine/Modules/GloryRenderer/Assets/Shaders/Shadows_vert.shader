#type vert

#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "internal/RenderConstants.glsl"
#include "internal/Camera.glsl"
#include "internal/PerObjectData.glsl"

#define FEATURE_TRANSPARENT_TEXTURED

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoord;
layout(location = 5) in vec4 inColor;

#ifdef WITH_TRANSPARENT_TEXTURED
layout(location = 0) out vec2 fragTexCoord;
#endif

void main()
{
	CameraData camera = CurrentCamera();

	mat4 world = WorldTransform();
	gl_Position = camera.Projection*camera.View*world*vec4(inPosition, 1.0);
#ifdef WITH_TRANSPARENT_TEXTURED
	fragTexCoord = inTexCoord;
#endif
}
