#type vert
#version 450

layout (location = 0) in vec3 InPosition;
layout (location = 0) out vec3 TexCoords;

#ifdef PUSH_CONSTANTS
layout(push_constant) uniform SkyboxRenderConstantsUBO
#else
layout(set = 0, std140, binding = 0) readonly uniform SkyboxRenderConstantsUBO
#endif
{
	uint CameraIndex;
} Constants;


#include "Internal/Camera.glsl"

void main()
{
    CameraData camera = CurrentCamera();
    TexCoords = InPosition;
    gl_Position = camera.Projection*mat4(mat3(camera.View))*vec4(InPosition, 1.0);
}
