#type vert

#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Internal/RenderConstants.glsl"
#include "Internal/PerObjectData.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoord;
layout(location = 5) in vec4 inColor;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec4 outColor;

void main()
{
	CameraData camera = CurrentCamera();
	mat4 world = WorldTransform();
	gl_Position = camera.Projection*camera.View*world*vec4(inPosition, 1.0);
	normal = vec3(world*vec4(inNormal, 0.0));
	outColor = inColor;
}
