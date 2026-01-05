#type vert
#version 450

layout (location = 0) in vec3 InPosition;
layout (location = 0) out vec3 TexCoords;

#ifdef PUSH_CONSTANTS
layout(push_constant) uniform IrradianceConstantsUBO
#else
layout(set = 0, std140, binding = 0) readonly uniform IrradianceConstantsUBO
#endif
{
	mat4 View;
	mat4 Projection;
} Constants;

void main()
{
    TexCoords = InPosition;
    gl_Position = Constants.Projection*mat4(mat3(Constants.View))*vec4(InPosition, 1.0);
}
