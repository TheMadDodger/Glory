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
	gl_Position = Cameras[Constants.CameraIndex].Projection*Cameras[Constants.CameraIndex].View*Worlds[Constants.ObjectDataIndex]*vec4(inPosition, 1.0);
	normal = vec3(Worlds[Constants.ObjectDataIndex]*vec4(inNormal, 0.0));
	outColor = inColor;
}
