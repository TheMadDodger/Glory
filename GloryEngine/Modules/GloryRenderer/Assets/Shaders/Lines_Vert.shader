#type vert
#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef PUSH_CONSTANTS
layout(push_constant) uniform LineRenderConstantsUBO
#else
layout(set = 0, std140, binding = 0) readonly uniform LineRenderConstantsUBO
#endif
{
	uint CameraIndex;
} Constants;

#include "Internal/Camera.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main()
{
	CameraData camera = CurrentCamera();

	gl_Position = camera.Projection*camera.View*vec4(inPosition, 1.0);
	outColor = inColor;
}
