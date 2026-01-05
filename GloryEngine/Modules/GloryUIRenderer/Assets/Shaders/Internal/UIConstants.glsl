#ifdef PUSH_CONSTANTS
layout(push_constant) uniform UIConstantsUBO
#else
layout(set = 0, std140, binding = 0) readonly uniform UIConstantsUBO
#endif
{
	mat4 Projection;
	uint ObjectIndex;
	uint ColorIndex;
	uint HasTexture;
} Constants;