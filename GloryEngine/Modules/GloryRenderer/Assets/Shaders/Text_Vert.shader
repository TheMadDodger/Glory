#type vert
#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Internal/RenderConstants.glsl"
#include "Internal/Camera.glsl"
#include "Internal/PerObjectData.glsl"

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 fragTexCoord;

void main()
{
	CameraData camera = CurrentCamera();
	mat4 world = WorldTransform();
	gl_Position = camera.Projection*camera.View*world*vec4(inPosition, 0.0, 1.0);
	outNormal = vec3(world*vec4(0.0, 0.0, 1.0, 0.0));
	fragTexCoord = inTexCoord;
	outColor = vec4(inColor, 1.0);
}
