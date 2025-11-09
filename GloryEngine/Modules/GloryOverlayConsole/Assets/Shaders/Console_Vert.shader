#type vert
#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef PUSH_CONSTANTS
layout(push_constant) uniform ConsoleRenderConstantsUBO
#else
layout(set = 0, std140, binding = 0) readonly uniform ConsoleRenderConstantsUBO
#endif
{
	mat4 Model;
	mat4 Projection;
} Constants;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outColor;

void main()
{
    gl_Position = Constants.Projection*Constants.Model*vec4(inPosition.xy, 0.0, 1.0);
    outTexCoord = inTexCoord;
	outColor = inColor;
}