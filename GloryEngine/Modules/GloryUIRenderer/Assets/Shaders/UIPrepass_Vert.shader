#type vert
#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Internal/UIConstants.glsl"
#include "Internal/UIData.glsl"

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec4 outColor;

void main()
{
    gl_Position = Constants.Projection*WorldTransform()*vec4(inPosition.xy, 0.0, 1.0);
    outTexCoord = inTexCoord;
	outColor = vec4(inColor, 1.0)*Color();
} 